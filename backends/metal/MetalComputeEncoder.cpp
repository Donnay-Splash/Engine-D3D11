#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalComputeEncoder::MetalComputeEncoder(std::shared_ptr<MetalCommandContext> context,
                                         MTL::ComputeCommandEncoder *encoder)
    : context_(std::move(context)), encoder_(NS::RetainPtr(encoder))
{
}

MetalComputeEncoder::~MetalComputeEncoder()
{
    Error ignored;
    End(ignored);
}

bool MetalComputeEncoder::End(Error &error)
{
    if (ended_) {
        SetError(error, "The Metal compute encoder has already ended.");
        return false;
    }
    encoder_->endEncoding();
    ended_ = true;
    context_->EndEncoder();
    return true;
}

bool MetalComputeEncoder::SetPipeline(const std::shared_ptr<ComputePipeline> &pipeline,
                                      Error &error)
{
    auto metalPipeline = std::dynamic_pointer_cast<MetalComputePipeline>(pipeline);
    if (ended_ || metalPipeline == nullptr || metalPipeline->Device() != context_->Device()) {
        SetError(error, "The compute pipeline is invalid or belongs to another Metal device.");
        return false;
    }
    encoder_->setComputePipelineState(metalPipeline->Native());
    pipeline_ = std::move(metalPipeline);
    context_->Keep(pipeline_);
    return true;
}

bool MetalComputeEncoder::SetBindingSet(std::uint32_t index,
                                        const std::shared_ptr<BindingSet> &bindings, Error &error)
{
    if (ended_ || index != 0 || bindings == nullptr || pipeline_ == nullptr ||
        pipeline_->Layout() == nullptr || bindings->Layout() != pipeline_->Layout()) {
        SetError(error, "The compute binding set index or object is invalid.");
        return false;
    }
    auto set = std::dynamic_pointer_cast<MetalBindingSet>(bindings);
    auto layout = std::dynamic_pointer_cast<MetalBindingLayout>(bindings->Layout());
    if (set == nullptr || layout == nullptr) {
        SetError(error, "The compute binding set is not owned by Metal.");
        return false;
    }
    for (const BindingDesc &binding : layout->Bindings()) {
        if ((static_cast<std::uint32_t>(binding.visibility) &
             static_cast<std::uint32_t>(ShaderStage::Compute)) == 0) {
            continue;
        }
        const std::vector<BindingValue> *values = set->Values(binding.binding);
        if (values == nullptr) {
            SetError(error, "The compute binding set is missing a declared binding.");
            return false;
        }
        for (std::uint32_t element = 0; element < binding.arrayCount; ++element) {
            const BindingValue &value = (*values)[element];
            const std::uint32_t slot = binding.binding + element;
            if (binding.type == BindingType::UniformBuffer ||
                binding.type == BindingType::StorageBuffer) {
                auto view = std::dynamic_pointer_cast<MetalBufferView>(value.buffer);
                if (view == nullptr || view->NativeBuffer() == nullptr) {
                    SetError(error, "A required compute buffer binding is unset.");
                    return false;
                }
                encoder_->setBuffer(view->NativeBuffer()->Native(), view->Offset(), slot);
                context_->Keep(value.buffer);
            } else if (binding.type == BindingType::SampledTexture ||
                       binding.type == BindingType::StorageTexture) {
                auto view = std::dynamic_pointer_cast<MetalTextureView>(value.texture);
                if (view == nullptr) {
                    SetError(error, "A required compute texture binding is unset.");
                    return false;
                }
                encoder_->setTexture(view->Native(), slot);
                context_->Keep(value.texture);
            } else {
                auto sampler = std::dynamic_pointer_cast<MetalSampler>(value.sampler);
                if (sampler == nullptr) {
                    SetError(error, "A required compute sampler binding is unset.");
                    return false;
                }
                encoder_->setSamplerState(sampler->Native(), slot);
                context_->Keep(value.sampler);
            }
        }
    }
    context_->Keep(bindings);
    return true;
}

void MetalComputeEncoder::Dispatch(std::uint32_t groupCountX, std::uint32_t groupCountY,
                                   std::uint32_t groupCountZ)
{
    if (ended_ || pipeline_ == nullptr || groupCountX == 0 || groupCountY == 0 ||
        groupCountZ == 0) {
        context_->DeferError(
            "A Metal dispatch requires an active encoder, pipeline, and nonzero group counts.");
        return;
    }
    const NS::UInteger width = pipeline_->Native()->maxTotalThreadsPerThreadgroup();
    encoder_->dispatchThreadgroups(MTL::Size::Make(groupCountX, groupCountY, groupCountZ),
                                   MTL::Size::Make(width, 1, 1));
}

} // namespace Metal
} // namespace Engine
