#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalGraphicsPipeline final : public GraphicsPipeline
{
public:
    MetalGraphicsPipeline(GraphicsPipelineDesc description, MTL::RenderPipelineState *pipeline,
                          MTL::DepthStencilState *depthState, MTL::Device *device);

    const GraphicsPipelineDesc &Description() const noexcept override;

    std::shared_ptr<BindingLayout> Layout() const override;

    MTL::RenderPipelineState *Native() const noexcept;

    MTL::DepthStencilState *DepthState() const noexcept;

    MTL::Device *Device() const noexcept;

private:
    GraphicsPipelineDesc description_;
    NS::SharedPtr<MTL::RenderPipelineState> pipeline_;
    NS::SharedPtr<MTL::DepthStencilState> depthState_;
    MTL::Device *device_;
};

} // namespace Metal
} // namespace Engine
