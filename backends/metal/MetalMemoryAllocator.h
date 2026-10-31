#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalMemoryAllocator final : public MemoryAllocator
{
public:
    explicit MetalMemoryAllocator(NS::SharedPtr<MTL::Device> device);

    MemoryRequirements GetRequirements(const BufferDesc &description) const override;

    MemoryRequirements GetRequirements(const TextureDesc &description) const override;

    std::shared_ptr<MemoryAllocation> Allocate(const MemoryRequirements &requirements,
                                               MemoryType type, Error &error) override;

private:
    NS::SharedPtr<MTL::Device> device_;
};

} // namespace Metal
} // namespace Engine
