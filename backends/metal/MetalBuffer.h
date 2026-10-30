#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalBuffer final : public Buffer
{
public:
    MetalBuffer(BufferDesc description, MTL::Buffer *buffer);

    const BufferDesc &Description() const noexcept override;

    MTL::Buffer *Native() const noexcept;

    bool Map(void *&data, Error &error) override;

    void Unmap() noexcept override;

private:
    BufferDesc description_;
    NS::SharedPtr<MTL::Buffer> buffer_;
};

} // namespace Metal
} // namespace Engine
