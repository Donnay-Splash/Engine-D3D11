#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalComputePipeline final : public ComputePipeline
{
public:
    MetalComputePipeline(ComputePipelineDesc description, MTL::ComputePipelineState *pipeline,
                         MTL::Device *device);

    const ComputePipelineDesc &Description() const noexcept override;

    std::shared_ptr<BindingLayout> Layout() const override;

    MTL::ComputePipelineState *Native() const noexcept;

    MTL::Device *Device() const noexcept;

private:
    ComputePipelineDesc description_;
    NS::SharedPtr<MTL::ComputePipelineState> pipeline_;
    MTL::Device *device_;
};

} // namespace Metal
} // namespace Engine
