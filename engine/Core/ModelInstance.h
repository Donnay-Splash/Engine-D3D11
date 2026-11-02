#pragma once

#include "Core/MaterialInstance.h"
#include "Core/Math.h"
#include "Core/Model.h"

#include <memory>
#include <vector>

namespace Engine {

// A Model placed in the world, with one MaterialInstance per model part.
class ModelInstance
{
public:
    explicit ModelInstance(std::shared_ptr<const Model> model);

    const std::shared_ptr<const Model> &GetModel() const noexcept { return m_Model; }

    void SetPosition(Maths::Vector3 position) noexcept { m_Position = position; }
    void SetOrientation(Maths::Quaternion orientation) noexcept { m_Orientation = orientation; }
    void SetScale(float uniformScale) noexcept { m_Scale = uniformScale; }
    Maths::Vector3 Position() const noexcept { return m_Position; }
    Maths::Quaternion Orientation() const noexcept { return m_Orientation; }
    float Scale() const noexcept { return m_Scale; }
    Maths::Matrix4 WorldMatrix() const;

    // Index matches Model::Parts().
    MaterialInstance &GetMaterialInstance(std::size_t partIndex);
    const MaterialInstance &GetMaterialInstance(std::size_t partIndex) const;

private:
    std::shared_ptr<const Model> m_Model;
    std::vector<MaterialInstance> m_MaterialInstances;
    Maths::Vector3 m_Position;
    Maths::Quaternion m_Orientation;
    float m_Scale;
};

} // namespace Engine
