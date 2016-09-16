#include "SceneNode.h"

template <class ComponentType>
inline std::shared_ptr<ComponentType> SceneNode::AddComponent(ID3D11Device* device)
{
    // Check that the given template argument is a derived class of Component
    static_assert(std::is_base_of<Component, ComponentType>::value, "A component must be derived from the Component class");

    auto newComponent = std::shared_ptr<ComponentType>(new ComponentType(shared_from_this()));
    newComponent->Initialize(device);
    m_components.push_back(newComponent);
    return newComponent;
}