#pragma once

#include "Core/Math.h"

#include <cstddef>
#include <type_traits>

namespace Engine {

// Per-draw uniform block; layout must match MeshUniforms in shaders/Scene.hlsl.
struct alignas(16) MeshTransforms
{
    Maths::Matrix4 world;
    Maths::Matrix4 view;
    Maths::Matrix4 projection;
};

static_assert(std::is_standard_layout<MeshTransforms>::value,
    "Transform uniforms must have standard layout.");
static_assert(sizeof(MeshTransforms) == sizeof(float) * 48,
    "Transform uniforms must contain three packed matrices.");
static_assert(offsetof(MeshTransforms, view) == sizeof(float) * 16,
    "View transform must immediately follow world transform.");
static_assert(offsetof(MeshTransforms, projection) == sizeof(float) * 32,
    "Projection transform must immediately follow view transform.");

} // namespace Engine
