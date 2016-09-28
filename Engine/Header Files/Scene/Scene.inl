
template <class ComponentType>
inline std::vector<std::shared_ptr<ComponentType>> Scene::GetAllComponentsOfType()
{
    static_assert(std::is_base_of<Component, ComponentType>::value, "A component must be derived from the Component class");

    std::vector<std::shared_ptr<ComponentType>> foundComponents;

    auto childNodes = m_rootNode->GetChildNodes();
    for (auto child : childNodes)
    {
        auto childComponents = GetAllComponentsOfType<ComponentType>(child);
        if (!childComponents.empty())
        {
            foundComponents.insert(foundComponents.end(), childComponents.begin(), childComponents.end());
        }
    }

    return foundComponents;
}

template <class ComponentType>
std::vector<std::shared_ptr<ComponentType>> Scene::GetAllComponentsOfType(SceneNode::Ptr sceneNode)
{
    static_assert(std::is_base_of<Component, ComponentType>::value, "A component must be derived from the Component class");
    
    // Ensure that the node is not the root.
    EngineAssert(!sceneNode->IsRootNode());

    std::vector<std::shared_ptr<ComponentType>> foundComponents;
    auto component = sceneNode->GetComponentOfType<ComponentType>();
    if (component != nullptr)
    {
        foundComponents.push_back(component);
    }
    auto childNodes = sceneNode->GetChildNodes();
    for (auto childNode : childNodes)
    {
        auto childComponents = GetAllComponentsOfType<ComponentType>(childNode);
        if (!childComponents.empty())
        {
            foundComponents.insert(foundComponents.end(), childComponents.begin(), childComponents.end());
        }
    }

    return foundComponents;
}
