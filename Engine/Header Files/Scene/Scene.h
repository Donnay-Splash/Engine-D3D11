#pragma once

#include "SceneNode.h"



class Scene : public std::enable_shared_from_this<Scene>
{
public:
    // Fired from the SceneNodeAddedEvent
    using SceneNodeAddedDelegate = std::function<void(SceneNode::Ptr nodeAdded)>;
    // Fired from the SceneNodeRemovedEvent
    using SceneNodeRemovedDelegate = std::function<void(SceneNode::Ptr nodeRemoved)>;
    using Ptr = std::shared_ptr<Scene>;

    Scene();
    Scene(const Scene&) = delete;
    ~Scene() {}

    void Initialize();

    SceneNode::Ptr AddNode(SceneNode::Ptr parentNode = nullptr);
    SceneNode::Ptr GetRootNode() const { return m_rootNode; }

    void Update(float frameTime);
    void Render(ID3D11DeviceContext* deviceContext);

    void RegisterSceneNodeAddedCallback(SceneNodeAddedDelegate callback);
    void RegisterSceneNodeRemovedCallback(SceneNodeRemovedDelegate callback);
    void FireSceneNodeAddedEvent(SceneNode::Ptr sceneNodeAdded);
    void FireSceneNodeRemovedEvent(SceneNode::Ptr sceneNodeRemoved);

    // Returns a vector containing all instances of ComponentType in the scene.
    template <class ComponentType>
    std::vector<std::shared_ptr<ComponentType>> GetAllComponentsOfType();

    // Returns a vector containing all instances of ComponentType in the subtree
    // starting from sceneNode.
    template <class ComponentType>
    std::vector<std::shared_ptr<ComponentType>> GetAllComponentsOfType(SceneNode::Ptr sceneNode);

    // Calculates the bounding box that fits all child nodes of the scene node.
    // If the SceneNode has no bounds component and no children the result bounding box will be empty.
    // If the SceneNode has no children but has a bounds component the result will be the bounds of the
    // current node.
    // If the SceneNode has children with bounds components the returned value will be the bounding box
    // that can contain all of these bounding boxes transformed into world space.
    static Utils::Maths::BoundingBox CalculateBoundingBoxForSceneNode(SceneNode::Ptr sceneNode);

private:
    // TODO: Need to ensure that no components are added to the root node.
    SceneNode::Ptr m_rootNode;
    std::vector<SceneNodeAddedDelegate> m_sceneNodeAddedCallbacks;
    std::vector<SceneNodeRemovedDelegate> m_sceneNodeRemovedCallbacks;
};

#include "Scene.inl"