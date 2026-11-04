#pragma once

#include "Core/Material.h"
#include "Core/MaterialValue.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Engine {

struct Error;

// Per-instance parameter overrides on top of a shared Material. Setters fail if the
// material has no parameter with that name or the value has the wrong type.
class MaterialInstance
{
public:
    explicit MaterialInstance(std::shared_ptr<const Material> material);

    const std::shared_ptr<const Material> &GetMaterial() const noexcept { return m_Material; }

    bool Set(const std::string &name, MaterialValue value, Error &error);
    bool SetBool(const std::string &name, bool value, Error &error);
    bool SetInt(const std::string &name, std::int32_t value, Error &error);
    bool SetFloat(const std::string &name, float value, Error &error);
    bool SetVector2(const std::string &name, Maths::Vector2 value, Error &error);
    bool SetVector3(const std::string &name, Maths::Vector3 value, Error &error);
    bool SetVector4(const std::string &name, Maths::Vector4 value, Error &error);
    bool SetTexture(const std::string &name, MaterialTexture value, Error &error);

    void ClearOverride(const std::string &name);
    bool IsOverridden(const std::string &name) const;

    // Override if present, otherwise the layout default.
    std::optional<MaterialValue> Get(const std::string &name) const;

    // Values for every parameter in layout order, defaults filled in.
    std::vector<MaterialValue> ResolveValues() const;

    // Increases whenever any value changes, so GPU copies can tell they are stale.
    std::uint64_t Version() const noexcept { return m_Version; }

private:
    std::shared_ptr<const Material> m_Material;
    std::vector<std::optional<MaterialValue>> m_Overrides;
    std::uint64_t m_Version = 1;
};

} // namespace Engine
