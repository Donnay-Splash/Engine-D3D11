#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalBufferView::MetalBufferView(std::shared_ptr<Buffer> buffer, std::size_t offset,
                                 std::size_t size)
    : buffer_(std::move(buffer)), offset_(offset), size_(size)
{
}

std::shared_ptr<Buffer> MetalBufferView::GetBuffer() const
{
    return buffer_;
}

std::size_t MetalBufferView::Offset() const noexcept
{
    return offset_;
}

std::size_t MetalBufferView::Size() const noexcept
{
    return size_;
}

MetalBuffer *MetalBufferView::NativeBuffer() const
{
    return dynamic_cast<MetalBuffer *>(buffer_.get());
}

} // namespace Metal
} // namespace Engine
