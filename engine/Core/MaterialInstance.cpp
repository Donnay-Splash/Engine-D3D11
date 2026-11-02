#include "MaterialInstance.h"

#include "Core/Types.h"

#include <cassert>
#include <utility>

namespace Engine {

MaterialInstance::MaterialInstance(std::shared_ptr<const Material> material)
    : m_Material(std::move(material))
{
    assert(m_Material != nullptr);
    m_Overrides.resize(m_Material->Parameters().size());
}

bool MaterialInstance::SetVector4(const std::string &name, Maths::Vector4 value, Error &error)
{
    const std::optional<std::size_t> index = m_Material->FindParameter(name);
    if (!index) {
        error.message = "The material has no parameter named '" + name + "'.";
        return false;
    }
    m_Overrides[*index] = value;
    return true;
}

void MaterialInstance::ClearOverride(const std::string &name)
{
    if (const std::optional<std::size_t> index = m_Material->FindParameter(name)) {
        m_Overrides[*index].reset();
    }
}

bool MaterialInstance::IsOverridden(const std::string &name) const
{
    const std::optional<std::size_t> index = m_Material->FindParameter(name);
    return index && m_Overrides[*index].has_value();
}

std::optional<Maths::Vector4> MaterialInstance::GetVector4(const std::string &name) const
{
    const std::optional<std::size_t> index = m_Material->FindParameter(name);
    if (!index) {
        return std::nullopt;
    }
    return m_Overrides[*index].value_or(m_Material->Parameters()[*index].defaultValue);
}

std::vector<Maths::Vector4> MaterialInstance::ResolveParameters() const
{
    std::vector<Maths::Vector4> values;
    values.reserve(m_Overrides.size());
    for (std::size_t index = 0; index < m_Overrides.size(); ++index) {
        values.push_back(
            m_Overrides[index].value_or(m_Material->Parameters()[index].defaultValue));
    }
    return values;
}

} // namespace Engine
