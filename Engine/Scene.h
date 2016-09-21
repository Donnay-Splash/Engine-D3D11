#pragma once

#include "SceneNode.h"



class Scene : public std::enable_shared_from_this<Scene>
{
public:
    using Ptr = std::shared_ptr<Scene>;
    Scene();
    Scene(const Scene&) = delete;
    ~Scene() {}

    void Initialize();

    SceneNode::Ptr AddNode(SceneNode::Ptr parentNode = nullptr);
    SceneNode::Ptr GetRootNode() const { return m_rootNode; }

    void Update(float frameTime);
    void Render(ID3D11DeviceContext* deviceContext);

    // Returns a vector containing all instances of ComponentType in the scene.
    template <class ComponentType>
    std::vector<std::shared_ptr<ComponentType>> GetAllComponentsOfType();

    // Returns a vector containing all instances of ComponentType in the subtree
    // starting from sceneNode.
    template <class ComponentType>
    std::vector<std::shared_ptr<ComponentType>> GetAllComponentsOfType(SceneNode::Ptr sceneNode);

private:
    // TODO: Need to ensure that no components are added to the root node.
    SceneNode::Ptr m_rootNode;
};

#include "Scene.inl"