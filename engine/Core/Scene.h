#pragma once

#include "Core/Math.h"
#include "Core/ModelInstance.h"

#include <memory>
#include <vector>

namespace Engine {

// View and projection used for a frame. Identity until a real camera exists.
struct Camera
{
    Maths::Matrix4 view = Maths::Matrix4::identity();
    Maths::Matrix4 projection = Maths::Matrix4::identity();
};

// The set of model instances the renderer iterates each frame.
class Scene
{
public:
    void Add(std::shared_ptr<ModelInstance> instance);
    bool Remove(const std::shared_ptr<ModelInstance> &instance);
    void Clear() noexcept { m_Instances.clear(); }

    const std::vector<std::shared_ptr<ModelInstance>> &Instances() const noexcept
    {
        return m_Instances;
    }
    Camera &GetCamera() noexcept { return m_Camera; }
    const Camera &GetCamera() const noexcept { return m_Camera; }

private:
    std::vector<std::shared_ptr<ModelInstance>> m_Instances;
    Camera m_Camera;
};

} // namespace Engine
