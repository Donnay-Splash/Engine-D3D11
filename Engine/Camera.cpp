#include "pch.h"
#include "Camera.h"
#include "SceneNode.h"

Camera::Camera(Component::SceneNodePtr sceneNode) : Component(sceneNode)
{
    // Set default camera properties.
    m_fov = Utils::Maths::DegreesToRadians(45.0f);
    m_projectionMode = ProjectionMode::Perspective;
    m_nearClip = 0.1f;
    m_farClip = 1000.0f;
}

Camera::~Camera()
{
}

void Camera::Initialize(ID3D11Device* device)
{
    m_viewConstants = std::make_shared<ConstantBuffer<ViewConstants>>(0, PipelineStage::Vertex, device);
}

void Camera::Update(float frameTime)
{

}

void Camera::Render(ID3D11DeviceContext* deviceContext) const
{
    auto sceneNode = GetSceneNode();
    auto transform = sceneNode->GetWorldTransform();
    auto viewMatrix = transform.GetInverse();

    EngineAssert(false); // Need to set render targets here.
    // Probably via d3d/renderer

}

void Camera::CalculateProjectionMatrix()
{
    // Implementation unfinished.
    EngineAssert(false);
    if (m_projectionMode == ProjectionMode::Perspective)
    {
        // aspect ratio needs to come from render target or viewport.
        auto aspectRatio = 0.0f;
        m_projectionMatrix = Utils::Maths::Matrix::CreatePerspectiveProjectionMatrix(m_fov, aspectRatio, m_nearClip, m_farClip);
    }
    else if (m_projectionMode == ProjectionMode::Orthographic)
    {
        // Need to get width and height from render target or something.
        float width = 0.0f;
        float height = 0.0f;
        m_projectionMatrix = Utils::Maths::Matrix::CreateOrthographicProjectionMatrix(width, height, m_nearClip, m_farClip);
    }

}

