#include "pch.h"
#include <Scene\SceneNode.h>

namespace Engine
{
    SceneNode::SceneNode(ScenePtr scene, bool isRoot /* = false*/) : SceneElement(L"Scene Node"),
        m_scene(scene), m_isRootNode(isRoot)
    {
        // Initialise scale to 1
        SetScale(1.0f);

        AddPublicProperties();
    }

    SceneNode::~SceneNode()
    {

    }

    void SceneNode::Update(float frameTime)
    {
        CalculateTransform();
        for (auto child : m_childNodes)
        {
            child->Update(frameTime);
        }

        for (auto component : m_components)
        {
            component->Update(frameTime);
        }
    }

    void SceneNode::Render(ID3D11DeviceContext* deviceContext)
    {
        for (auto child : m_childNodes)
        {
            child->Render(deviceContext);
        }

        for (auto component : m_components)
        {
            component->Render(deviceContext);
        }
    }

    void SceneNode::RegisterComponentAddedCallback(ComponentAddedDelegate callback)
    {
        EngineAssert(callback != nullptr);
        m_componentAddedCallbacks.push_back(callback);
    }

    void SceneNode::FireComponentAddedCallback(Component::Ptr componentAdded)
    {
        for (auto callback : m_componentAddedCallbacks)
        {
            callback(componentAdded);
        }
    }

    void SceneNode::SetPosition(Utils::Maths::Vector3 position, bool forceTransformUpdate /*= false*/)
    {
        m_position = position;
        m_transformDirty = true;
        if (forceTransformUpdate)
        {
            CalculateTransform();
        }
    }

    void SceneNode::SetScale(float scale, bool forceTransformUpdate /*= false*/)
    {
        m_scale = { scale, scale, scale };
        m_transformDirty = true;
        if (forceTransformUpdate)
        {
            CalculateTransform();
        }
    }

    void SceneNode::SetRotation(Utils::Maths::Quaternion rotation, bool forceTransformUpdate /*= false*/)
    {
        m_rotation = rotation;
        m_transformDirty = true;
        if (forceTransformUpdate)
        {
            CalculateTransform();
        }
    }

    void SceneNode::SetTransform(Utils::Maths::Matrix transform)
    {
        // TODO: Check for finite affine transformation matrix.
        m_transform = transform;
        std::tie(m_position, m_rotation, m_scale) = m_transform.Decompose();
    }

    void SceneNode::CalculateTransform()
    {
        if (m_transformDirty)
        {
            m_transform = Utils::Maths::Matrix::CreateFromTranslationRotationScale(m_position, m_rotation, m_scale.x);
            m_transformDirty = false;
        }
    }

    void SceneNode::AddPublicProperties()
    {
        //// TODO: Add helpers to try and cut this down to 1 line in most cases
        //auto positionSetter = [this](Utils::Maths::Vector4 v) {m_position = { v.x, v.y, v.z }; };
        //auto positionGetter = [this]() { return Utils::Maths::Vector4(m_position.x, m_position.y, m_position.z, 0.0f); };
        //auto positionProperty = std::make_shared<Property>("Position", PropertyType::Vector, positionSetter, positionGetter);
        //AddProperty(positionProperty);

    }


    Utils::Maths::Matrix SceneNode::GetWorldTransform() const
    {
        if (m_parentNode)
        {
            return GetTransform() * m_parentNode->GetWorldTransform();
        }
        else
        {
            return GetTransform();
        }
    }

    Utils::Maths::Vector3 SceneNode::GetPosition() const
    {
        return m_position;
    }

    Utils::Maths::Vector3 SceneNode::GetWorldSpacePosition() const
    {
        if (m_parentNode)
        {
            return m_position * m_parentNode->GetWorldTransform();
        }
        return m_position;
    }

    SceneNode::Ptr SceneNode::GetSharedThis()
    {
        return std::static_pointer_cast<SceneNode>(shared_from_this());
    }
}