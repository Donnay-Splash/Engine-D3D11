#pragma once

#include "SceneNode.h"
#include <Resources\ShaderPipeline.h>

namespace Engine
{
    class Scene : public std::enable_shared_from_this<Scene>
    {
    public:
        using Ptr = std::shared_ptr<Scene>;

        Scene();
        Scene(const Scene&) = delete;
        ~Scene() {}

        void Initialize();

        /*  Adds a new node to the scene. Specifying the parent node and the name for the node.
            If the parent pointer is left null. The SceneNode will be parented to the rootNode
            The name can be used to aid debugging. Making it easier to recognise the purpose of
            a node. By default the name is set to "Scene Node" */
        SceneNode::Ptr AddNode(SceneNode::Ptr parentNode = nullptr, std::wstring name = L"Scene Node");
        SceneNode::Ptr GetRootNode() const { return m_rootNode; }

        void Update(float frameTime);
        void Render(ShaderPipeline::Ptr shaderOverride = nullptr);

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

        Utils::Maths::BoundingBox GetSceneBounds() const { return m_sceneBounds; }

        // HACK HACK HACK HACK HACK
        // TODO: Find a better way to access constants like this.
        Utils::Maths::Matrix GetCameraTransform() const { return m_cameraTransform; }
        Utils::Maths::Matrix GetWorldToCameraTransform() const { return m_invCameraTransform; }
        Utils::Maths::Matrix GetPrevCameraTransform() const { return m_prevCameraTransform; }
        Utils::Maths::Matrix GetPreviousWorldToCameraTransform() const { return m_prevInvCameraTransform; }
        // TODO: Find a better way to modify constants like this.
        void SetCameraTransform(const Utils::Maths::Matrix& cameraTransform);

        /*  Called by SceneNodes and components to notify the scene that it has changed*/
        void NotifySceneChanged() { m_sceneChangedThisFrame = true; }

    private:
        // TODO: Need to ensure that no components are added to the root node.
        SceneNode::Ptr m_rootNode;

        Utils::Maths::BoundingBox m_sceneBounds;
        bool m_sceneChangedThisFrame = true;

        // HACK HACK HACK HACK HACK
        // TODO: Find a better way of storing scene constants like this.
        Utils::Maths::Matrix m_cameraTransform;
        Utils::Maths::Matrix m_invCameraTransform;
        Utils::Maths::Matrix m_prevCameraTransform;
        Utils::Maths::Matrix m_prevInvCameraTransform;
    };

#include "Scene.inl" 
}