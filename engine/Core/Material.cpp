#include "Material.h"

#include <utility>

namespace Engine {

Material::Material(
    std::string name,
    std::vector<MaterialParameter> parameters,
    Graphics::CullMode cullMode,
    Graphics::PrimitiveTopology topology)
    : m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_CullMode(cullMode),
      m_Topology(topology)
{
}

std::shared_ptr<Material> Material::CreateFlatColor(std::string name, Maths::Vector4 baseColor)
{
    return std::make_shared<Material>(
        std::move(name),
        std::vector<MaterialParameter>{{kBaseColor, baseColor}});
}

std::optional<std::size_t> Material::FindParameter(const std::string &name) const noexcept
{
    for (std::size_t index = 0; index < m_Parameters.size(); ++index) {
        if (m_Parameters[index].name == name) {
            return index;
        }
    }
    return std::nullopt;
}

} // namespace Engine
