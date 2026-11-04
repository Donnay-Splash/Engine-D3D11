#pragma once

#include "Core/MaterialLayout.h"
#include "Graphics/GraphicsTypes.h"

#include <memory>
#include <string>

namespace Engine {

// Shared, immutable description of how a surface is shaded: a shader program, the
// parameters that program exposes, and fixed pipeline state. Per-instance changes go
// through MaterialInstance.
class Material
{
public:
    static constexpr const char *kDefaultShaderProgram = "Mesh";

    // `layout` comes from the program's ShaderProgram::materialLayout; null for programs
    // with no material parameters.
    Material(
        std::string name,
        std::shared_ptr<const MaterialLayout> layout = nullptr,
        std::string shaderProgram = kDefaultShaderProgram,
        Graphics::CullMode cullMode = Graphics::CullMode::None,
        Graphics::PrimitiveTopology topology = Graphics::PrimitiveTopology::TriangleList);

    const std::string &Name() const noexcept { return m_Name; }
    const std::shared_ptr<const MaterialLayout> &Layout() const noexcept { return m_Layout; }
    Graphics::CullMode CullMode() const noexcept { return m_CullMode; }
    Graphics::PrimitiveTopology Topology() const noexcept { return m_Topology; }
    // Name of a program in ShaderPrograms.cfg.
    const std::string &ShaderProgram() const noexcept { return m_ShaderProgram; }

    std::size_t ParameterCount() const noexcept
    {
        return m_Layout == nullptr ? 0 : m_Layout->Parameters().size();
    }

private:
    std::string m_Name;
    std::shared_ptr<const MaterialLayout> m_Layout;
    Graphics::CullMode m_CullMode;
    Graphics::PrimitiveTopology m_Topology;
    std::string m_ShaderProgram;
};

} // namespace Engine
