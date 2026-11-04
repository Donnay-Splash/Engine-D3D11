#include "MaterialInstance.h"

#include "Core/Types.h"

#include <atomic>
#include <cassert>
#include <utility>

namespace Engine {

namespace {

std::uint64_t nextVersion()
{
    static std::atomic<std::uint64_t> s_Counter{0};
    return ++s_Counter;
}

} // namespace

MaterialInstance::MaterialInstance(std::shared_ptr<const Material> material)
    : m_Material(std::move(material))
{
    assert(m_Material != nullptr);
    m_Overrides.resize(m_Material->ParameterCount());
    m_Version = nextVersion();
}

bool MaterialInstance::Set(const std::string &name, MaterialValue value, Error &error)
{
    const auto &layout = m_Material->Layout();
    const std::optional<std::size_t> index =
        layout == nullptr ? std::nullopt : layout->Find(name);
    if (!index) {
        error.message = "The material has no parameter named '" + name + "'.";
        return false;
    }
    if (layout->Parameters()[*index].type != TypeOf(value)) {
        error.message = "The value has the wrong type for material parameter '" + name + "'.";
        return false;
    }
    m_Overrides[*index] = std::move(value);
    m_Version = nextVersion();
    return true;
}

bool MaterialInstance::SetBool(const std::string &name, bool value, Error &error)
{
    return Set(name, MaterialValue(std::in_place_type<bool>, value), error);
}

bool MaterialInstance::SetInt(const std::string &name, std::int32_t value, Error &error)
{
    return Set(name, MaterialValue(std::in_place_type<std::int32_t>, value), error);
}

bool MaterialInstance::SetFloat(const std::string &name, float value, Error &error)
{
    return Set(name, MaterialValue(std::in_place_type<float>, value), error);
}

bool MaterialInstance::SetVector2(const std::string &name, Maths::Vector2 value, Error &error)
{
    return Set(name, MaterialValue(value), error);
}

bool MaterialInstance::SetVector3(const std::string &name, Maths::Vector3 value, Error &error)
{
    return Set(name, MaterialValue(value), error);
}

bool MaterialInstance::SetVector4(const std::string &name, Maths::Vector4 value, Error &error)
{
    return Set(name, MaterialValue(value), error);
}

bool MaterialInstance::SetTexture(const std::string &name, MaterialTexture value, Error &error)
{
    return Set(name, MaterialValue(std::move(value)), error);
}

void MaterialInstance::ClearOverride(const std::string &name)
{
    const auto &layout = m_Material->Layout();
    if (layout == nullptr) {
        return;
    }
    if (const std::optional<std::size_t> index = layout->Find(name)) {
        if (m_Overrides[*index].has_value()) {
            m_Overrides[*index].reset();
            m_Version = nextVersion();
        }
    }
}

bool MaterialInstance::IsOverridden(const std::string &name) const
{
    const auto &layout = m_Material->Layout();
    const std::optional<std::size_t> index =
        layout == nullptr ? std::nullopt : layout->Find(name);
    return index && m_Overrides[*index].has_value();
}

std::optional<MaterialValue> MaterialInstance::Get(const std::string &name) const
{
    const auto &layout = m_Material->Layout();
    const std::optional<std::size_t> index =
        layout == nullptr ? std::nullopt : layout->Find(name);
    if (!index) {
        return std::nullopt;
    }
    return m_Overrides[*index].value_or(layout->Parameters()[*index].defaultValue);
}

std::vector<MaterialValue> MaterialInstance::ResolveValues() const
{
    std::vector<MaterialValue> values;
    values.reserve(m_Overrides.size());
    for (std::size_t index = 0; index < m_Overrides.size(); ++index) {
        values.push_back(m_Overrides[index].value_or(
            m_Material->Layout()->Parameters()[index].defaultValue));
    }
    return values;
}

} // namespace Engine
