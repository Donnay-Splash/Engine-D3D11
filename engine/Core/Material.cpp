#include "Material.h"

#include <utility>

namespace Engine {

Material::Material(
    std::string name,
    std::shared_ptr<const MaterialLayout> layout,
    std::string shaderProgram,
    Graphics::CullMode cullMode,
    Graphics::PrimitiveTopology topology)
    : m_Name(std::move(name)),
      m_Layout(std::move(layout)),
      m_CullMode(cullMode),
      m_Topology(topology),
      m_ShaderProgram(std::move(shaderProgram))
{
}

} // namespace Engine
