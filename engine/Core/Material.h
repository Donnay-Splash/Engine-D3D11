#pragma once

#include "Core/Math.h"
#include "Graphics/GraphicsTypes.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Engine {

struct MaterialParameter
{
    std::string name;
    Maths::Vector4 defaultValue;
};

// Shared, immutable description of how a surface is shaded. Per-instance changes
// go through MaterialInstance.
class Material
{
public:
    static constexpr const char *kBaseColor = "baseColor";
    static constexpr const char *kDefaultShaderProgram = "Mesh";

    Material(
        std::string name,
        std::vector<MaterialParameter> parameters,
        Graphics::CullMode cullMode = Graphics::CullMode::None,
        Graphics::PrimitiveTopology topology = Graphics::PrimitiveTopology::TriangleList,
        std::string shaderProgram = kDefaultShaderProgram);

    static std::shared_ptr<Material> CreateFlatColor(
        std::string name, Maths::Vector4 baseColor);

    const std::string &Name() const noexcept { return m_Name; }
    const std::vector<MaterialParameter> &Parameters() const noexcept { return m_Parameters; }
    Graphics::CullMode CullMode() const noexcept { return m_CullMode; }
    Graphics::PrimitiveTopology Topology() const noexcept { return m_Topology; }
    // Name of a program in ShaderPrograms.cfg.
    const std::string &ShaderProgram() const noexcept { return m_ShaderProgram; }
    std::optional<std::size_t> FindParameter(const std::string &name) const noexcept;

private:
    std::string m_Name;
    std::vector<MaterialParameter> m_Parameters;
    Graphics::CullMode m_CullMode;
    Graphics::PrimitiveTopology m_Topology;
    std::string m_ShaderProgram;
};

} // namespace Engine
