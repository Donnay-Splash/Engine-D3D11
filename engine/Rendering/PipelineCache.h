#pragma once

#include "Core/Mesh.h"
#include "Core/Types.h"
#include "Graphics/GraphicsPipeline.h"
#include "Rendering/ShaderProgram.h"
#include "ShaderLibrary.h"
#include "ShaderProgramCatalog.h"

#include <compare>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Engine {
namespace Graphics {
class Device;
}

// The only pipeline state not derivable from the program and the mesh.
struct PipelineState
{
    Graphics::CullMode cullMode = Graphics::CullMode::None;
    Graphics::PrimitiveTopology topology = Graphics::PrimitiveTopology::TriangleList;
    std::vector<Graphics::TextureFormat> colorFormats;
    Graphics::TextureFormat depthStencilFormat = Graphics::TextureFormat::Unknown;
    std::uint32_t sampleCount = 1;
};

// Resolves programs from the shader library on first use and builds pipelines lazily from
// a program, the vertex streams of a mesh and a PipelineState. Pipelines are never created
// up front because they depend on that runtime state.
class PipelineCache
{
public:
    bool Initialize(
        std::shared_ptr<Graphics::Device> device,
        std::shared_ptr<ShaderLibrary> shaderLibrary,
        Error &error);
    void Shutdown() noexcept;

    std::shared_ptr<const ShaderProgram> GetProgram(const std::string &name, Error &error);

    // Only the mesh streams the program's vertex shader reads are put in the pipeline.
    std::shared_ptr<Graphics::GraphicsPipeline> GetPipeline(
        const ShaderProgram &program,
        const Mesh &mesh,
        const PipelineState &state,
        Error &error);

    std::size_t PipelineCount() const noexcept { return m_Pipelines.size(); }

private:
    struct PipelineKey
    {
        std::string program;
        // Flattened vertex attributes and buffers.
        std::vector<std::uint32_t> vertexLayout;
        Graphics::CullMode cullMode;
        Graphics::PrimitiveTopology topology;
        std::vector<Graphics::TextureFormat> colorFormats;
        Graphics::TextureFormat depthStencilFormat;
        std::uint32_t sampleCount;

        auto operator<=>(const PipelineKey &) const = default;
    };

    std::shared_ptr<ShaderProgram> ResolveProgram(const ShaderProgramDesc &desc, Error &error);

    std::shared_ptr<Graphics::Device> m_Device;
    std::shared_ptr<ShaderLibrary> m_ShaderLibrary;
    ShaderProgramCatalog m_Catalog;
    std::map<std::string, std::shared_ptr<ShaderProgram>> m_Programs;
    std::map<PipelineKey, std::shared_ptr<Graphics::GraphicsPipeline>> m_Pipelines;
};

} // namespace Engine
