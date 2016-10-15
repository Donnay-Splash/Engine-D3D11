#pragma once
#include <Scene\Components\Component.h>
#include <Scene\Public Properties\SceneElement.h>

namespace Engine
{
    // Forward declaration
    class Scene;

    class SceneNode : public SceneElement
    {
    private:
        using ScenePtr = std::shared_ptr<Scene>;
        // Fired from the component added event
        // TODO: Move to weak pointers
        using ComponentAddedDelegate = std::function<void(Component::Ptr component)>;
    public:
        using Ptr = std::shared_ptr<SceneNode>;
        using SceneNodeContainer = std::vector<SceneNode::Ptr>;
        SceneNode(const SceneNode&) = delete;
        ~SceneNode();

        void Update(float frameTime);
        void Render(ID3D11DeviceContext* deviceContext);

        void RegisterComponentAddedCallback(ComponentAddedDelegate callback);
        void FireComponentAddedCallback(Component::Ptr componentAdded);

        inline ComponentContainer GetComponents() const { return m_components; }
        inline SceneNodeContainer GetChildNodes() const { return m_childNodes; }

        SceneNode::Ptr GetParentNode() { return m_parentNode; }

        template <class ComponentType>
        inline std::shared_ptr<ComponentType> GetComponentOfType();

        // Temporary change. Move device somewhere else
        template <class ComponentType>
        std::shared_ptr<ComponentType> AddComponent(ID3D11Device* device);

        void SetPosition(Utils::Maths::Vector3 position, bool forceTransformUpdate = false);
        // For now only support uniform scale
        void SetScale(float scale, bool forceTransformUpdate = false);
        void SetRotation(Utils::Maths::Quaternion rotation, bool forceTransformUpdate = false);

        void SetTransform(Utils::Maths::Matrix transform);

        Utils::Maths::Matrix GetTransform() const { return m_transform; }
        Utils::Maths::Matrix GetWorldTransform() const;

        Utils::Maths::Vector3 GetPosition() const;
        Utils::Maths::Vector3 GetWorldSpacePosition() const;

        std::shared_ptr<Scene> GetScene() const { return m_scene; }

        bool IsRootNode() const { return m_isRootNode; }

    private:
        SceneNode(ScenePtr scene, bool isRoot = false);
        static Ptr Create(ScenePtr scene, bool isRoot = false) { return std::shared_ptr<SceneNode>(new SceneNode(scene, isRoot)); }

        Ptr GetSharedThis();

        void CalculateTransform();
        void AddPublicProperties();

        friend class Scene;
    private:
        // The scene this node is attached to.
        ScenePtr m_scene;

        ComponentContainer m_components;
        SceneNodeContainer m_childNodes;
        SceneNode::Ptr m_parentNode;

        Utils::Maths::Vector3 m_position;
        Utils::Maths::Vector3 m_scale;
        Utils::Maths::Quaternion m_rotation;
        Utils::Maths::Matrix m_transform;
        bool m_transformDirty = false;
        std::vector<ComponentAddedDelegate> m_componentAddedCallbacks;

        // Flag to signal if this SceneNode is the root of a scene.
        bool m_isRootNode;
    };

#include "SceneNode.inl" 
}