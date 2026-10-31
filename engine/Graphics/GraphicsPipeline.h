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
    std::shared_ptr<BindingLayout> bindingLayout;
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
    virtual const GraphicsPipelineDesc &Description() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
