#pragma once

#include "GraphicsTypes.h"
#include "Pipeline.h"
#include "Shader.h"

#include <memory>
#include <vector>

namespace Engine {
namespace Graphics {

struct GraphicsPipelineDesc
{
    std::vector<std::shared_ptr<Shader>> shaders;
    // One layout per binding set, indexed by set number. Empty entries are unused sets.
    std::vector<std::shared_ptr<BindingLayout>> bindingLayouts;
    std::vector<VertexAttributeDesc> vertexAttributes;
    std::vector<VertexBufferLayoutDesc> vertexBuffers;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    CullMode cullMode = CullMode::Back;
    DepthStencilState depthStencil;
    std::vector<TextureFormat> colorFormats;
    TextureFormat depthStencilFormat = TextureFormat::Unknown;
    std::uint32_t sampleCount = 1;
};

class GraphicsPipeline : public Pipeline
{
public:
    using Pipeline::Layout;

    virtual const GraphicsPipelineDesc &Description() const noexcept = 0;

    // Layout of the given set, or null when the pipeline does not use it.
    std::shared_ptr<BindingLayout> Layout(std::uint32_t set) const
    {
        const auto &layouts = Description().bindingLayouts;
        return set < layouts.size() ? layouts[set] : nullptr;
    }
};

} // namespace Graphics
} // namespace Engine
