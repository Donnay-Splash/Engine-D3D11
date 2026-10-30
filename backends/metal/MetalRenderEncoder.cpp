#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalRenderEncoder::MetalRenderEncoder(std::shared_ptr<MetalCommandContext> context,
                                       MTL::RenderCommandEncoder *encoder,
                                       std::vector<TextureFormat> colorFormats,
                                       TextureFormat depthFormat, std::uint32_t sampleCount)
    : context_(std::move(context)), encoder_(NS::RetainPtr(encoder)),
      colorFormats_(std::move(colorFormats)), depthFormat_(depthFormat), sampleCount_(sampleCount)
{
}

MetalRenderEncoder::~MetalRenderEncoder()
{
    Error ignored;
    End(ignored);
}

bool MetalRenderEncoder::End(Error &error)
{
    if (ended_) {
        SetError(error, "The Metal render encoder has already ended.");
        return false;
    }
    encoder_->endEncoding();
    ended_ = true;
    context_->EndEncoder();
    return true;
}

bool MetalRenderEncoder::SetPipeline(const std::shared_ptr<GraphicsPipeline> &pipeline,
                                     Error &error)
{
    auto metalPipeline = std::dynamic_pointer_cast<MetalGraphicsPipeline>(pipeline);
    if (ended_ || metalPipeline == nullptr || metalPipeline->Device() != context_->Device()) {
        SetError(error, "The graphics pipeline is invalid or belongs to another Metal device.");
        return false;
    }
    const GraphicsPipelineDesc &description = metalPipeline->Description();
    if (description.colorFormats != colorFormats_ ||
        description.depthStencilFormat != depthFormat_ || description.sampleCount != sampleCount_) {
        SetError(error, "The graphics pipeline attachment formats do not match the render pass.");
        return false;
    }
    encoder_->setRenderPipelineState(metalPipeline->Native());
    encoder_->setDepthStencilState(metalPipeline->DepthState());
    switch (description.cullMode) {
    case CullMode::None:
        encoder_->setCullMode(MTL::CullModeNone);
        break;
    case CullMode::Front:
        encoder_->setCullMode(MTL::CullModeFront);
        break;
    case CullMode::Back:
        encoder_->setCullMode(MTL::CullModeBack);
        break;
    }
    pipeline_ = std::move(metalPipeline);
    context_->Keep(pipeline_);
    return true;
}

bool MetalRenderEncoder::SetBindingSet(std::uint32_t index,
                                       const std::shared_ptr<BindingSet> &bindings, Error &error)
{
    if (ended_ || pipeline_ == nullptr || index != 0 || bindings == nullptr ||
        pipeline_->Layout() == nullptr || bindings->Layout() != pipeline_->Layout()) {
        SetError(error, "The render binding set is invalid or does not match the pipeline layout.");
        return false;
    }
    auto metalSet = std::dynamic_pointer_cast<MetalBindingSet>(bindings);
    auto metalLayout = std::dynamic_pointer_cast<MetalBindingLayout>(bindings->Layout());
    if (metalSet == nullptr || metalLayout == nullptr) {
        SetError(error, "The binding set is not owned by the Metal backend.");
        return false;
    }
    for (const BindingDesc &binding : metalLayout->Bindings()) {
        if ((static_cast<std::uint32_t>(binding.visibility) &
             static_cast<std::uint32_t>(ShaderStage::Vertex | ShaderStage::Fragment)) == 0) {
            continue;
        }
        const std::vector<BindingValue> *values = metalSet->Values(binding.binding);
        if (values == nullptr) {
            SetError(error, "The render binding set is missing a declared binding.");
            return false;
        }
        for (std::uint32_t element = 0; element < binding.arrayCount; ++element) {
            const BindingValue &value = (*values)[element];
            const std::uint32_t slot = binding.binding + element;
            if (binding.type == BindingType::UniformBuffer ||
                binding.type == BindingType::StorageBuffer) {
                auto view = std::dynamic_pointer_cast<MetalBufferView>(value.buffer);
                if (view == nullptr || view->NativeBuffer() == nullptr) {
                    SetError(error, "A required render buffer binding is unset.");
                    return false;
                }
                const std::uint32_t visibility = static_cast<std::uint32_t>(binding.visibility);
                if ((visibility & static_cast<std::uint32_t>(ShaderStage::Vertex)) != 0) {
                    encoder_->setVertexBuffer(view->NativeBuffer()->Native(), view->Offset(), slot);
                }
                if ((visibility & static_cast<std::uint32_t>(ShaderStage::Fragment)) != 0) {
                    encoder_->setFragmentBuffer(view->NativeBuffer()->Native(), view->Offset(),
                                                slot);
                }
                context_->Keep(value.buffer);
            } else if (binding.type == BindingType::SampledTexture ||
                       binding.type == BindingType::StorageTexture) {
                auto view = std::dynamic_pointer_cast<MetalTextureView>(value.texture);
                if (view == nullptr) {
                    SetError(error, "A required render texture binding is unset.");
                    return false;
                }
                const std::uint32_t visibility = static_cast<std::uint32_t>(binding.visibility);
                if ((visibility & static_cast<std::uint32_t>(ShaderStage::Vertex)) != 0) {
                    encoder_->setVertexTexture(view->Native(), slot);
                }
                if ((visibility & static_cast<std::uint32_t>(ShaderStage::Fragment)) != 0) {
                    encoder_->setFragmentTexture(view->Native(), slot);
                }
                context_->Keep(value.texture);
            } else {
                auto sampler = std::dynamic_pointer_cast<MetalSampler>(value.sampler);
                if (sampler == nullptr) {
                    SetError(error, "A required render sampler binding is unset.");
                    return false;
                }
                const std::uint32_t visibility = static_cast<std::uint32_t>(binding.visibility);
                if ((visibility & static_cast<std::uint32_t>(ShaderStage::Vertex)) != 0) {
                    encoder_->setVertexSamplerState(sampler->Native(), slot);
                }
                if ((visibility & static_cast<std::uint32_t>(ShaderStage::Fragment)) != 0) {
                    encoder_->setFragmentSamplerState(sampler->Native(), slot);
                }
                context_->Keep(value.sampler);
            }
        }
    }
    context_->Keep(bindings);
    return true;
}

bool MetalRenderEncoder::SetViewport(const Viewport &viewport, Error &error)
{
    if (ended_ || viewport.width <= 0.0f || viewport.height <= 0.0f || !std::isfinite(viewport.x) ||
        !std::isfinite(viewport.y) || !std::isfinite(viewport.width) ||
        !std::isfinite(viewport.height) || !std::isfinite(viewport.minDepth) ||
        !std::isfinite(viewport.maxDepth) || viewport.minDepth < 0.0f || viewport.maxDepth > 1.0f ||
        viewport.minDepth > viewport.maxDepth) {
        SetError(error, "The Metal viewport is invalid.");
        return false;
    }
    encoder_->setViewport(MTL::Viewport{viewport.x, viewport.y, viewport.width, viewport.height,
                                        viewport.minDepth, viewport.maxDepth});
    return true;
}

bool MetalRenderEncoder::SetVertexBuffer(std::uint32_t slot, const std::shared_ptr<Buffer> &buffer,
                                         std::size_t offset, Error &error)
{
    auto metalBuffer = std::dynamic_pointer_cast<MetalBuffer>(buffer);
    if (ended_ || metalBuffer == nullptr ||
        !HasBits(static_cast<std::uint32_t>(metalBuffer->Description().usage),
                 static_cast<std::uint32_t>(BufferUsage::Vertex)) ||
        offset >= metalBuffer->Description().size) {
        SetError(error, "The vertex buffer or offset is invalid.");
        return false;
    }
    encoder_->setVertexBuffer(metalBuffer->Native(), offset, slot);
    context_->Keep(buffer);
    return true;
}

bool MetalRenderEncoder::SetIndexBuffer(const std::shared_ptr<Buffer> &buffer, std::size_t offset,
                                        IndexFormat format, Error &error)
{
    auto metalBuffer = std::dynamic_pointer_cast<MetalBuffer>(buffer);
    const std::size_t alignment = format == IndexFormat::UInt16 ? 2 : 4;
    if (ended_ || metalBuffer == nullptr ||
        (format != IndexFormat::UInt16 && format != IndexFormat::UInt32) ||
        !HasBits(static_cast<std::uint32_t>(metalBuffer->Description().usage),
                 static_cast<std::uint32_t>(BufferUsage::Index)) ||
        offset >= metalBuffer->Description().size || offset % alignment != 0) {
        SetError(error, "The index buffer or offset is invalid.");
        return false;
    }
    indexBuffer_ = std::move(metalBuffer);
    indexOffset_ = offset;
    indexFormat_ = format;
    context_->Keep(buffer);
    return true;
}

void MetalRenderEncoder::Draw(std::uint32_t vertexCount, std::uint32_t instanceCount,
                              std::uint32_t firstVertex, std::uint32_t firstInstance)
{
    if (ended_ || pipeline_ == nullptr || vertexCount == 0 || instanceCount == 0) {
        context_->DeferError(
            "A Metal draw call requires an active pass, pipeline, and nonzero counts.");
        return;
    }
    encoder_->drawPrimitives(ToMetal(pipeline_->Description().topology), firstVertex, vertexCount,
                             instanceCount, firstInstance);
}

void MetalRenderEncoder::DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount,
                                     std::uint32_t firstIndex, std::int32_t baseVertex,
                                     std::uint32_t firstInstance)
{
    if (ended_ || pipeline_ == nullptr || indexBuffer_ == nullptr || indexCount == 0 ||
        instanceCount == 0) {
        context_->DeferError("A Metal indexed draw requires an active pass, pipeline, index "
                             "buffer, and nonzero counts.");
        return;
    }
    const std::size_t stride = indexFormat_ == IndexFormat::UInt16 ? 2 : 4;
    const std::size_t offset = indexOffset_ + static_cast<std::size_t>(firstIndex) * stride;
    if (offset > indexBuffer_->Description().size ||
        static_cast<std::size_t>(indexCount) * stride > indexBuffer_->Description().size - offset) {
        context_->DeferError("The Metal indexed draw exceeds the index buffer.");
        return;
    }
    encoder_->drawIndexedPrimitives(
        ToMetal(pipeline_->Description().topology), indexCount,
        indexFormat_ == IndexFormat::UInt16 ? MTL::IndexTypeUInt16 : MTL::IndexTypeUInt32,
        indexBuffer_->Native(), offset, instanceCount, baseVertex, firstInstance);
}

} // namespace Metal
} // namespace Engine
