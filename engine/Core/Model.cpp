#include "Model.h"

#include "Core/Types.h"

#include <utility>

namespace Engine {

bool Model::AddPart(ModelPart part, Error &error)
{
    if (part.mesh == nullptr || part.material == nullptr) {
        error.message = "A model part requires a mesh and a material.";
        return false;
    }
    const Maths::Bounds &meshBounds = part.mesh->LocalBounds();
    if (!meshBounds.empty()) {
        m_Bounds.include(meshBounds.minimum());
        m_Bounds.include(meshBounds.maximum());
    }
    m_Parts.push_back(std::move(part));
    return true;
}

} // namespace Engine
