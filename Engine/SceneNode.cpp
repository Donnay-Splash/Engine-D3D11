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

Utils::Maths::Matrix SceneNode::GetTransform()
{
    return Utils::Maths::Matrix::CreateFromTranslationRotationScale(m_position, m_rotation, m_scale.x);
}

Utils::Maths::Matrix SceneNode::GetWorldTransform()
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