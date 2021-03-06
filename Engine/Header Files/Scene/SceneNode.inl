#include "SceneNode.h"

template <class ComponentType>
inline std::shared_ptr<ComponentType> SceneNode::AddComponent()
{
    // Check that the given template argument is a derived class of Component
    static_assert(std::is_base_of<Component, ComponentType>::value, "A component must be derived from the Component class");

    auto device = m_scene->GetDevice();

    // Check to see that there are no other components of ComponentType
    // attached to the SceneNode already.
    auto testComponent = GetComponentOfType<ComponentType>();
    EngineAssert(testComponent == nullptr);

    auto newComponent = std::shared_ptr<ComponentType>(new ComponentType(GetSharedThis()));
    newComponent->Initialize(device->GetDevice());
    m_components.push_back(newComponent);
    // Add the component as a child of this SceneElement so that it appears in the UI.
    AddChildElement(newComponent);
    FireComponentAddedCallback(newComponent);
    return newComponent;
}

template <class ComponentType>
inline std::shared_ptr<ComponentType> SceneNode::GetComponentOfType()
{
    static_assert(std::is_base_of<Component, ComponentType>::value, "A component must be derived from the Component class");

    // Go through each of the components to find one of type ComponentType
    // If a matching component is not found then return nullptr.
    for (auto component : m_components)
    {
        auto castComponent = std::dynamic_pointer_cast<ComponentType>(component);
        if (castComponent != nullptr)
        {
            return castComponent;
        }
    }

    return nullptr;
}