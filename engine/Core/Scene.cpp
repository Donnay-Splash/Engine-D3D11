#include "Scene.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace Engine {

void Scene::Add(std::shared_ptr<ModelInstance> instance)
{
    assert(instance != nullptr);
    m_Instances.push_back(std::move(instance));
}

bool Scene::Remove(const std::shared_ptr<ModelInstance> &instance)
{
    const auto found = std::find(m_Instances.begin(), m_Instances.end(), instance);
    if (found == m_Instances.end()) {
        return false;
    }
    m_Instances.erase(found);
    return true;
}

} // namespace Engine
