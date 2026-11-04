#pragma once

#include "Core/Math.h"
#include "Graphics/TextureView.h"

#include <cstdint>
#include <memory>
#include <variant>

namespace Engine {

enum class MaterialValueType
{
    Bool,
    Int,
    Float,
    Float2,
    Float3,
    Float4,
    Texture,
};

// Engine-provided textures a material texture parameter can default to.
enum class DefaultTexture
{
    White,
    Black,
    FlatNormal,
};

// A null texture means "use the parameter's default texture".
using MaterialTexture = std::shared_ptr<Graphics::TextureView>;

// Alternatives are in MaterialValueType order, so index() is the value's type.
using MaterialValue = std::variant<
    bool,
    std::int32_t,
    float,
    Maths::Vector2,
    Maths::Vector3,
    Maths::Vector4,
    MaterialTexture>;

inline MaterialValueType TypeOf(const MaterialValue &value) noexcept
{
    return static_cast<MaterialValueType>(value.index());
}

} // namespace Engine
