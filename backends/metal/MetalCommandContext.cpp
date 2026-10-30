#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalCommandContext::MetalCommandContext(std::shared_ptr<MetalQueue> queue)
    : queue_(std::move(queue))
{
    MTL::CommandBuffer *native = queue_->Native()->commandBuffer();
    if (native != nullptr) {
        commandBuffer_ = std::make_shared<MetalCommandBuffer>(queue_->Native(), native);
    }
}

bool MetalCommandContext::Valid() const noexcept
{
    return commandBuffer_ != nullptr;
}

std::shared_ptr<CommandBuffer> MetalCommandContext::Finish(Error &error)
{
    if (commandBuffer_ == nullptr || finished_) {
        SetError(error, "The Metal command context is invalid or already finished.");
        return nullptr;
    }
    if (activeEncoder_) {
        SetError(error, "End the active Metal command encoder before finishing the context.");
        return nullptr;
    }
    if (!deferredError_.empty()) {
        error.message = deferredError_;
        return nullptr;
    }
    commandBuffer_->MarkFinished();
    finished_ = true;
    return commandBuffer_;
}

bool MetalCommandContext::BeginEncoder(Error &error)
{
    if (commandBuffer_ == nullptr || finished_ || activeEncoder_) {
        SetError(error, "A Metal command encoder cannot begin in the current context state.");
        return false;
    }
    activeEncoder_ = true;
    return true;
}

void MetalCommandContext::EndEncoder()
{
    activeEncoder_ = false;
}

void MetalCommandContext::Keep(const std::shared_ptr<Resource> &resource)
{
    commandBuffer_->AddResource(resource);
    auto view = std::dynamic_pointer_cast<MetalTextureView>(resource);
    if (view != nullptr) {
        commandBuffer_->AddDrawable(view->Drawable());
    }
}

void MetalCommandContext::DeferError(const char *message)
{
    if (deferredError_.empty()) {
        deferredError_ = message;
    }
}

MTL::Device *MetalCommandContext::Device() const noexcept
{
    return queue_->Device();
}

MTL::CommandBuffer *MetalCommandContext::Native() const noexcept
{
    return commandBuffer_->Native();
}

QueueType MetalCommandContext::Type() const noexcept
{
    return queue_->Type();
}

std::unique_ptr<RenderPassEncoder>
MetalCommandContext::BeginRenderPass(const RenderPassDesc &description, Error &error)
{
    AutoreleasePool pool;
    if (Type() != QueueType::Graphics) {
        SetError(error, "Render passes require a Metal graphics queue.");
        return nullptr;
    }
    if (description.colorAttachments.size() > 8 ||
        (description.colorAttachments.empty() && !description.hasDepthStencilAttachment) ||
        description.extent.width == 0 || description.extent.height == 0 ||
        description.extent.depth != 1) {
        SetError(error, "The Metal render pass has an invalid extent or attachment count.");
        return nullptr;
    }

    std::vector<TextureFormat> colorFormats;
    std::uint32_t sampleCount = 0;
    for (const RenderAttachmentDesc &attachment : description.colorAttachments) {
        auto view = std::dynamic_pointer_cast<MetalTextureView>(attachment.view);
        if (view == nullptr || view->Format() == TextureFormat::Depth32Float ||
            view->Range().layerCount != 1 ||
            (attachment.loadAction != LoadAction::Load &&
             attachment.loadAction != LoadAction::Clear &&
             attachment.loadAction != LoadAction::DontCare) ||
            (attachment.storeAction != StoreAction::Store &&
             attachment.storeAction != StoreAction::DontCare) ||
            !std::isfinite(attachment.clearColor[0]) || !std::isfinite(attachment.clearColor[1]) ||
            !std::isfinite(attachment.clearColor[2]) || !std::isfinite(attachment.clearColor[3]) ||
            !HasBits(static_cast<std::uint32_t>(view->GetTexture()->Description().usage),
                     static_cast<std::uint32_t>(TextureUsage::ColorAttachment))) {
            SetError(error, "A color attachment is not a valid Metal color texture view.");
            return nullptr;
        }
        const TextureDesc &texture = view->GetTexture()->Description();
        const std::uint32_t mip = view->Range().firstMip;
        if (description.extent.width > std::max(1u, texture.extent.width >> mip) ||
            description.extent.height > std::max(1u, texture.extent.height >> mip) ||
            (sampleCount != 0 && sampleCount != texture.sampleCount)) {
            SetError(error,
                     "The render extent or sample count is incompatible with a color attachment.");
            return nullptr;
        }
        sampleCount = texture.sampleCount;
        colorFormats.push_back(view->Format());
    }

    TextureFormat depthFormat = TextureFormat::Unknown;
    std::shared_ptr<MetalTextureView> depthView;
    if (description.hasDepthStencilAttachment) {
        depthView =
            std::dynamic_pointer_cast<MetalTextureView>(description.depthStencilAttachment.view);
        if (depthView == nullptr || depthView->Format() != TextureFormat::Depth32Float ||
            depthView->Range().layerCount != 1 ||
            (description.depthStencilAttachment.loadAction != LoadAction::Load &&
             description.depthStencilAttachment.loadAction != LoadAction::Clear &&
             description.depthStencilAttachment.loadAction != LoadAction::DontCare) ||
            (description.depthStencilAttachment.storeAction != StoreAction::Store &&
             description.depthStencilAttachment.storeAction != StoreAction::DontCare) ||
            !std::isfinite(description.depthStencilAttachment.clearDepth) ||
            description.depthStencilAttachment.clearDepth < 0.0f ||
            description.depthStencilAttachment.clearDepth > 1.0f ||
            !HasBits(static_cast<std::uint32_t>(depthView->GetTexture()->Description().usage),
                     static_cast<std::uint32_t>(TextureUsage::DepthStencilAttachment))) {
            SetError(error, "The depth attachment is not a valid Metal depth texture view.");
            return nullptr;
        }
        depthFormat = depthView->Format();
        const TextureDesc &texture = depthView->GetTexture()->Description();
        const std::uint32_t mip = depthView->Range().firstMip;
        if (description.extent.width > std::max(1u, texture.extent.width >> mip) ||
            description.extent.height > std::max(1u, texture.extent.height >> mip) ||
            (sampleCount != 0 && sampleCount != texture.sampleCount)) {
            SetError(
                error,
                "The render extent or sample count is incompatible with the depth attachment.");
            return nullptr;
        }
        sampleCount = texture.sampleCount;
    }

    if (!BeginEncoder(error)) {
        return nullptr;
    }
    NS::SharedPtr<MTL::RenderPassDescriptor> nativePass =
        NS::RetainPtr(MTL::RenderPassDescriptor::renderPassDescriptor());
    for (std::size_t index = 0; index < description.colorAttachments.size(); ++index) {
        const RenderAttachmentDesc &attachment = description.colorAttachments[index];
        auto view = std::dynamic_pointer_cast<MetalTextureView>(attachment.view);
        MTL::RenderPassColorAttachmentDescriptor *color =
            nativePass->colorAttachments()->object(index);
        color->setTexture(view->Native());
        color->setLoadAction(attachment.loadAction == LoadAction::Load ? MTL::LoadActionLoad
                             : attachment.loadAction == LoadAction::Clear
                                 ? MTL::LoadActionClear
                                 : MTL::LoadActionDontCare);
        color->setStoreAction(attachment.storeAction == StoreAction::Store
                                  ? MTL::StoreActionStore
                                  : MTL::StoreActionDontCare);
        color->setClearColor(
            MTL::ClearColor::Make(attachment.clearColor[0], attachment.clearColor[1],
                                  attachment.clearColor[2], attachment.clearColor[3]));
        Keep(attachment.view);
    }
    if (depthView != nullptr) {
        const RenderAttachmentDesc &attachment = description.depthStencilAttachment;
        MTL::RenderPassDepthAttachmentDescriptor *depth = nativePass->depthAttachment();
        depth->setTexture(depthView->Native());
        depth->setLoadAction(attachment.loadAction == LoadAction::Load ? MTL::LoadActionLoad
                             : attachment.loadAction == LoadAction::Clear
                                 ? MTL::LoadActionClear
                                 : MTL::LoadActionDontCare);
        depth->setStoreAction(attachment.storeAction == StoreAction::Store
                                  ? MTL::StoreActionStore
                                  : MTL::StoreActionDontCare);
        depth->setClearDepth(attachment.clearDepth);
        Keep(attachment.view);
    }

    MTL::RenderCommandEncoder *encoder = Native()->renderCommandEncoder(nativePass.get());
    if (encoder == nullptr) {
        EndEncoder();
        SetError(error, "Metal failed to create a render command encoder.");
        return nullptr;
    }
    return std::unique_ptr<RenderPassEncoder>(new MetalRenderEncoder(
        shared_from_this(), encoder, std::move(colorFormats), depthFormat, sampleCount));
}

std::unique_ptr<ComputeEncoder> MetalCommandContext::BeginCompute(Error &error)
{
    AutoreleasePool pool;
    if (Type() == QueueType::Copy) {
        SetError(error, "Compute encoders are unavailable on a Metal copy queue.");
        return nullptr;
    }
    if (!BeginEncoder(error)) {
        return nullptr;
    }
    MTL::ComputeCommandEncoder *encoder = Native()->computeCommandEncoder();
    if (encoder == nullptr) {
        EndEncoder();
        SetError(error, "Metal failed to create a compute command encoder.");
        return nullptr;
    }
    return std::unique_ptr<ComputeEncoder>(new MetalComputeEncoder(shared_from_this(), encoder));
}

std::unique_ptr<CopyEncoder> MetalCommandContext::BeginCopy(Error &error)
{
    AutoreleasePool pool;
    if (!BeginEncoder(error)) {
        return nullptr;
    }
    MTL::BlitCommandEncoder *encoder = Native()->blitCommandEncoder();
    if (encoder == nullptr) {
        EndEncoder();
        SetError(error, "Metal failed to create a blit command encoder.");
        return nullptr;
    }
    return std::unique_ptr<CopyEncoder>(new MetalCopyEncoder(shared_from_this(), encoder));
}

bool MetalCommandContext::Barrier(const std::shared_ptr<Resource> &resource,
                                  const ResourceBarrierDesc &description, Error &error)
{
    if (resource == nullptr || finished_ || commandBuffer_ == nullptr) {
        SetError(error, "The Metal resource barrier has no valid resource or context.");
        return false;
    }
    auto buffer = std::dynamic_pointer_cast<MetalBuffer>(resource);
    auto texture = std::dynamic_pointer_cast<MetalTexture>(resource);
    if (buffer == nullptr && texture == nullptr) {
        SetError(error, "Metal barriers support only backend-owned buffers and textures.");
        return false;
    }

    const auto usageSupported = [buffer, texture](ResourceUsage usage) {
        if (usage == ResourceUsage::Undefined) {
            return true;
        }
        if (buffer != nullptr) {
            const std::uint32_t flags = static_cast<std::uint32_t>(buffer->Description().usage);
            switch (usage) {
            case ResourceUsage::CopySource:
                return HasBits(flags, static_cast<std::uint32_t>(BufferUsage::CopySource));
            case ResourceUsage::CopyDestination:
                return HasBits(flags, static_cast<std::uint32_t>(BufferUsage::CopyDestination));
            case ResourceUsage::VertexBuffer:
                return HasBits(flags, static_cast<std::uint32_t>(BufferUsage::Vertex));
            case ResourceUsage::IndexBuffer:
                return HasBits(flags, static_cast<std::uint32_t>(BufferUsage::Index));
            case ResourceUsage::UniformBuffer:
                return HasBits(flags, static_cast<std::uint32_t>(BufferUsage::Uniform));
            case ResourceUsage::ShaderRead:
            case ResourceUsage::ShaderWrite:
                return HasBits(flags, static_cast<std::uint32_t>(BufferUsage::Storage));
            default:
                return false;
            }
        }
        const std::uint32_t flags = static_cast<std::uint32_t>(texture->Description().usage);
        switch (usage) {
        case ResourceUsage::CopySource:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::CopySource));
        case ResourceUsage::CopyDestination:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::CopyDestination));
        case ResourceUsage::ShaderRead:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::Sampled));
        case ResourceUsage::ShaderWrite:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::Storage));
        case ResourceUsage::ColorAttachment:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::ColorAttachment));
        case ResourceUsage::DepthStencilAttachment:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::DepthStencilAttachment));
        case ResourceUsage::Present:
            return HasBits(flags, static_cast<std::uint32_t>(TextureUsage::ColorAttachment));
        default:
            return false;
        }
    };
    if (!usageSupported(description.before) || !usageSupported(description.after)) {
        SetError(error, "The Metal barrier usage is incompatible with the resource.");
        return false;
    }
    if (texture != nullptr && (description.range.firstMip >= texture->Description().mipLevels ||
                               description.range.mipCount == 0 ||
                               description.range.mipCount >
                                   texture->Description().mipLevels - description.range.firstMip ||
                               description.range.firstLayer >= texture->Description().arrayLayers ||
                               description.range.layerCount == 0 ||
                               description.range.layerCount > texture->Description().arrayLayers -
                                                                  description.range.firstLayer)) {
        SetError(error, "The Metal resource barrier range is invalid.");
        return false;
    }
    // Metal's tracked resources perform hazard transitions implicitly in command order.
    Keep(resource);
    return true;
}

} // namespace Metal
} // namespace Engine
