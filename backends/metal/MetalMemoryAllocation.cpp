#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalMemoryAllocation::MetalMemoryAllocation(std::size_t size, MemoryType type, MTL::Buffer *buffer)
    : size_(size), type_(type), buffer_(NS::RetainPtr(buffer))
{
}

std::size_t MetalMemoryAllocation::Size() const noexcept
{
    return size_;
}

MemoryType MetalMemoryAllocation::Type() const noexcept
{
    return type_;
}

} // namespace Metal
} // namespace Engine
