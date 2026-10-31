#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalBufferView final : public BufferView
{
public:
    MetalBufferView(std::shared_ptr<Buffer> buffer, std::size_t offset, std::size_t size);

    std::shared_ptr<Buffer> GetBuffer() const override;

    std::size_t Offset() const noexcept override;

    std::size_t Size() const noexcept override;

    MetalBuffer *NativeBuffer() const;

private:
    std::shared_ptr<Buffer> buffer_;
    std::size_t offset_;
    std::size_t size_;
};

} // namespace Metal
} // namespace Engine
