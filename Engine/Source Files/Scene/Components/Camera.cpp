#include "pch.h"
#include <Scene\Components\Camera.h>
#include <Scene\SceneNode.h>

namespace Engine
{
    Camera::Camera(Component::SceneNodePtr sceneNode) : Component(sceneNode, L"Camera")
    {
        // Set default camera properties.
        m_fov = Utils::Maths::DegreesToRadians(45.0f);
        m_projectionMode = ProjectionMode::Perspective;
        m_nearClip = 0.1f;
        m_farClip = 1000.0f;
        m_aspectRatio = 0.0f;
        m_orthographicSize = 20.0f;

        RegisterPublicProperties();
    }

    Camera::~Camera()
    {
    }

    void Camera::Initialize(ID3D11Device* device)
    {
        m_viewConstants = std::make_shared<ConstantBuffer<ViewConstants>>(PipelineStage::Vertex, device);
    }

    void Camera::Update(float frameTime)
    {

    }

    void Camera::Render(ID3D11DeviceContext* deviceContext) const
    {
    }

    void Camera::Render(D3DClass::Ptr d3dClass, Scene::Ptr scene)
    {
        auto sceneNode = GetSceneNode();
        auto transform = sceneNode->GetWorldTransform();
        auto viewMatrix = transform.GetInverse();

        // TODO: Fix this temp hack
        if (m_renderTargetBundle == nullptr)
        {
            auto screenSize = d3dClass->GetScreenSize();
            m_aspectRatio = screenSize.x / screenSize.y;
        }
        CalculateProjectionMatrix();
        ViewConstants viewConstants;
        viewConstants.view = viewMatrix;
        viewConstants.projection = m_projectionMatrix;
        viewConstants.cameraPosition = sceneNode->GetWorldSpacePosition();

        m_viewConstants->SetData(viewConstants);
        m_viewConstants->UploadData(d3dClass->GetDeviceContext());
        d3dClass->SetRenderTarget(m_renderTargetBundle);

        scene->Render(d3dClass->GetDeviceContext());
    }

    void Camera::CalculateProjectionMatrix()
    {
        if (m_projectionMode == ProjectionMode::Perspective)
        {
            // aspect ratio needs to come from render target or viewport.
            m_projectionMatrix = Utils::Maths::Matrix::CreatePerspectiveProjectionMatrix(m_fov, m_aspectRatio, m_nearClip, m_farClip);
        }
        else if (m_projectionMode == ProjectionMode::Orthographic)
        {
            EngineAssert(m_orthographicSize > 0.0f);
            EngineAssert(m_aspectRatio > 0.0f);
            float sizeX = m_orthographicSize * m_aspectRatio;
            float sizeY = m_orthographicSize;
            m_projectionMatrix = Utils::Maths::Matrix::CreateOrthographicProjectionMatrix(sizeX, sizeY, m_nearClip, m_farClip);
        }

    }

    void Camera::SetRenderTargetBundle(RenderTargetBundle::Ptr renderTargetBundle)
    {
        m_renderTargetBundle = renderTargetBundle;
        if (m_renderTargetBundle == nullptr)
        {
            m_aspectRatio = 0.0f;
        }
        else
        {
            auto height = static_cast<float>(m_renderTargetBundle->GetHeight());
            auto width = static_cast<float>(m_renderTargetBundle->GetWidth());
            m_aspectRatio = width / height;
        }
    }

    void Camera::RegisterPublicProperties()
    {
        //// TODO: Add helpers to try and cut this down to 1 line in most cases
        auto fovSetter = [this](float v) 
        {
            // Convert value from degrees to radians
            auto radians = Utils::Maths::DegreesToRadians(v);
            SetFieldOfView(radians);
        };
        auto fovGetter = [this]()
        { 
            // convert field of view to degrees
            return Utils::Maths::RadiansToDegrees(m_fov);
        };
        RegisterScalarProperty(L"Field of View", fovGetter, fovSetter, 5.0, 180.0);

        auto nearPlaneGetter = [this]() { return m_nearClip; };
        auto nearPlaneSetter = [this](float v) { m_nearClip = v; };
        RegisterScalarProperty(L"Near Plane", nearPlaneGetter, nearPlaneSetter, 0.0f, 100.0f);

        auto farPlaneGetter = [this]() { return m_farClip; };
        auto farPlaneSetter = [this](float v) { m_farClip = v; };
        RegisterScalarProperty(L"Far Plane", farPlaneGetter, farPlaneSetter, 50.0f, 1000.0f);

        auto orthoSizeGetter = [this]() { return m_orthographicSize; };
        auto orthoSizeSetter = [this](float v) { m_orthographicSize = v; };
        RegisterScalarProperty(L"Orthographic Size", orthoSizeGetter, orthoSizeSetter, 1.0f, 1000.0f);

    }
}
