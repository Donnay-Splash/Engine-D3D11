#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalComputePipeline::MetalComputePipeline(ComputePipelineDesc description,
                                           MTL::ComputePipelineState *pipeline, MTL::Device *device)
    : description_(std::move(description)), pipeline_(NS::RetainPtr(pipeline)), device_(device)
{
}

const ComputePipelineDesc &MetalComputePipeline::Description() const noexcept
{
    return description_;
}

std::shared_ptr<BindingLayout> MetalComputePipeline::Layout() const
{
    return description_.bindingLayout;
}

MTL::ComputePipelineState *MetalComputePipeline::Native() const noexcept
{
    return pipeline_.get();
}

MTL::Device *MetalComputePipeline::Device() const noexcept
{
    return device_;
}

} // namespace Metal
} // namespace Engine
