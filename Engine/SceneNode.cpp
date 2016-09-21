#include "pch.h"
#include "SceneNode.h"

SceneNode::SceneNode(ScenePtr scene) :
    m_scene(scene)
{
    // Initialise scale to 1
    SetScale(1.0f);
}

SceneNode::~SceneNode()
{

}

void SceneNode::Update(float frameTime)
{
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

Utils::Maths::Matrix SceneNode::GetTransform() const
{
    return Utils::Maths::Matrix::CreateFromTranslationRotationScale(m_position, m_rotation, m_scale.x);
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