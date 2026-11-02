#pragma once

#include "Core/Material.h"
#include "Core/Math.h"
#include "Core/Mesh.h"

#include <memory>
#include <vector>

namespace Engine {

struct ModelPart
{
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<const Material> material;
};

// Shared, immutable-after-build set of mesh/material parts. Place it in the world
// with a ModelInstance.
class Model
{
public:
    // Parts need a mesh and a material.
    bool AddPart(ModelPart part, Error &error);

    const std::vector<ModelPart> &Parts() const noexcept { return m_Parts; }
    const Maths::Bounds &LocalBounds() const noexcept { return m_Bounds; }

private:
    std::vector<ModelPart> m_Parts;
    Maths::Bounds m_Bounds;
};

} // namespace Engine
