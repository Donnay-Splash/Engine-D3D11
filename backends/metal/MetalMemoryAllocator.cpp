#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalMemoryAllocator::MetalMemoryAllocator(NS::SharedPtr<MTL::Device> device)
    : device_(std::move(device))
{
}

MemoryRequirements MetalMemoryAllocator::GetRequirements(const BufferDesc &description) const
{
    return {description.size, 256};
}

MemoryRequirements MetalMemoryAllocator::GetRequirements(const TextureDesc &description) const
{
    std::size_t total = 0;
    std::size_t width = description.extent.width;
    std::size_t height = description.extent.height;
    std::size_t depth = description.extent.depth;
    const std::size_t pixelSize = BytesPerPixel(description.format);
    for (std::uint32_t mip = 0; mip < description.mipLevels; ++mip) {
        std::size_t level = 0;
        if (!CheckedMultiply(width, height, level) || !CheckedMultiply(level, depth, level) ||
            !CheckedMultiply(level, pixelSize, level) ||
            !CheckedMultiply(level, description.arrayLayers, level) ||
            !CheckedMultiply(level, description.sampleCount, level) ||
            total > std::numeric_limits<std::size_t>::max() - level) {
            return {0, 256};
        }
        total += level;
        width = std::max<std::size_t>(1, width / 2);
        height = std::max<std::size_t>(1, height / 2);
        depth = std::max<std::size_t>(1, depth / 2);
    }
    return {total, 256};
}

std::shared_ptr<MemoryAllocation>
MetalMemoryAllocator::Allocate(const MemoryRequirements &requirements, MemoryType type,
                               Error &error)
{
    AutoreleasePool pool;
    if (requirements.size == 0 || requirements.size > device_->maxBufferLength()) {
        SetError(error, "The requested Metal allocation size is invalid.");
        return nullptr;
    }
    MTL::Buffer *buffer = device_->newBuffer(requirements.size, ResourceOptionsFor(type));
    if (buffer == nullptr) {
        SetError(error, "Metal failed to allocate the requested memory.");
        return nullptr;
    }
    return std::make_shared<MetalMemoryAllocation>(requirements.size, type, buffer);
}

} // namespace Metal
} // namespace Engine
