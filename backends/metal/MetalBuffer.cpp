#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalBuffer::MetalBuffer(BufferDesc description, MTL::Buffer *buffer)
    : description_(description), buffer_(NS::RetainPtr(buffer))
{
}

const BufferDesc &MetalBuffer::Description() const noexcept
{
    return description_;
}

MTL::Buffer *MetalBuffer::Native() const noexcept
{
    return buffer_.get();
}

bool MetalBuffer::Map(void *&data, Error &error)
{
    data = nullptr;
    if (description_.memoryType == MemoryType::DeviceLocal) {
        SetError(error, "Private Metal buffers cannot be CPU mapped.");
        return false;
    }
    data = buffer_->contents();
    if (data == nullptr) {
        SetError(error, "Metal did not provide CPU-visible buffer contents.");
        return false;
    }
    return true;
}

void MetalBuffer::Unmap() noexcept {}

} // namespace Metal
} // namespace Engine
