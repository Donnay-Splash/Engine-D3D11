#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalMemoryAllocation final : public MemoryAllocation
{
public:
    MetalMemoryAllocation(std::size_t size, MemoryType type, MTL::Buffer *buffer);

    std::size_t Size() const noexcept override;

    MemoryType Type() const noexcept override;

private:
    std::size_t size_;
    MemoryType type_;
    NS::SharedPtr<MTL::Buffer> buffer_;
};

} // namespace Metal
} // namespace Engine
