#pragma once
#include "pch.h"
#include "Component.h"

// Forward declaration
class Scene;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
private:
    using ScenePtr = std::shared_ptr<Scene>;

public:
    using Ptr = std::shared_ptr<SceneNode>;
    using SceneNodeContainer = std::vector<SceneNode::Ptr>;
    SceneNode(const SceneNode&) = delete;
    ~SceneNode();

    inline ComponentContainer GetComponents() const { return m_components; }
    inline SceneNodeContainer GetChildNodes() const { return m_childNodes; }

    // Temporary change. Move device somewhere else
    template <class ComponentType>
    std::shared_ptr<ComponentType> AddComponent(ID3D11Device* device);

    void SetPosition(Utils::Maths::Vector3 position) { m_position = position; }
    // For now only support uniform scale
    void SetScale(float scale) { m_scale = {scale, scale, scale}; }
    void SetRotation(Utils::Maths::Quaternion rotation) { m_rotation = rotation; }

    Utils::Maths::Matrix GetTransform();
    Utils::Maths::Matrix GetWorldTransform();

    void Update(float frameTime);
    void Render(ID3D11DeviceContext* deviceContext);

    std::shared_ptr<Scene> GetScene() const { return m_scene; }

private:
    SceneNode(ScenePtr scene);
    static Ptr Create(ScenePtr scene) { return std::shared_ptr<SceneNode>(new SceneNode(scene)); }

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

};

#include "SceneNode.inl"