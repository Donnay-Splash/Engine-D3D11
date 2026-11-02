#include "ModelInstance.h"

#include <cassert>
#include <utility>

namespace Engine {

ModelInstance::ModelInstance(std::shared_ptr<const Model> model)
    : m_Model(std::move(model)),
      m_Position(0.0f, 0.0f, 0.0f),
      m_Orientation(0.0f, 0.0f, 0.0f, 1.0f),
      m_Scale(1.0f)
{
    assert(m_Model != nullptr);
    m_MaterialInstances.reserve(m_Model->Parts().size());
    for (const ModelPart &part : m_Model->Parts()) {
        m_MaterialInstances.emplace_back(part.material);
    }
}

Maths::Matrix4 ModelInstance::WorldMatrix() const
{
    return Maths::Matrix4::transform(m_Position, m_Orientation, m_Scale);
}

MaterialInstance &ModelInstance::GetMaterialInstance(std::size_t partIndex)
{
    assert(partIndex < m_MaterialInstances.size());
    return m_MaterialInstances[partIndex];
}

const MaterialInstance &ModelInstance::GetMaterialInstance(std::size_t partIndex) const
{
    assert(partIndex < m_MaterialInstances.size());
    return m_MaterialInstances[partIndex];
}

} // namespace Engine
