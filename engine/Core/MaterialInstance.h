#pragma once

#include "Core/Material.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Engine {

struct Error;

// Per-instance parameter overrides on top of a shared Material.
class MaterialInstance
{
public:
    explicit MaterialInstance(std::shared_ptr<const Material> material);

    const std::shared_ptr<const Material> &GetMaterial() const noexcept { return m_Material; }

    // Fails if the material has no parameter with this name.
    bool SetVector4(const std::string &name, Maths::Vector4 value, Error &error);
    void ClearOverride(const std::string &name);
    bool IsOverridden(const std::string &name) const;

    // Override if present, otherwise the material default.
    std::optional<Maths::Vector4> GetVector4(const std::string &name) const;

    // Values for every material parameter in schema order, defaults filled in.
    std::vector<Maths::Vector4> ResolveParameters() const;

private:
    std::shared_ptr<const Material> m_Material;
    std::vector<std::optional<Maths::Vector4>> m_Overrides;
};

} // namespace Engine
