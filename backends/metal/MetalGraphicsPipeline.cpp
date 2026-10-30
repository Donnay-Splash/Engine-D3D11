#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalGraphicsPipeline::MetalGraphicsPipeline(GraphicsPipelineDesc description,
                                             MTL::RenderPipelineState *pipeline,
                                             MTL::DepthStencilState *depthState,
                                             MTL::Device *device)
    : description_(std::move(description)), pipeline_(NS::RetainPtr(pipeline)),
      depthState_(NS::RetainPtr(depthState)), device_(device)
{
}

const GraphicsPipelineDesc &MetalGraphicsPipeline::Description() const noexcept
{
    return description_;
}

std::shared_ptr<BindingLayout> MetalGraphicsPipeline::Layout() const
{
    return description_.bindingLayout;
}

MTL::RenderPipelineState *MetalGraphicsPipeline::Native() const noexcept
{
    return pipeline_.get();
}

MTL::DepthStencilState *MetalGraphicsPipeline::DepthState() const noexcept
{
    return depthState_.get();
}

MTL::Device *MetalGraphicsPipeline::Device() const noexcept
{
    return device_;
}

} // namespace Metal
} // namespace Engine
