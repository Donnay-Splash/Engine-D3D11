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
        m_viewConstants = std::make_shared<ConstantBuffer<ViewConstants>>(PipelineStage::Vertex | PipelineStage::Pixel, device);
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
        Utils::Maths::Vector2 viewSize;

        // TODO: Fix this temp hack
        if (m_renderTargetBundle == nullptr)
        {
            viewSize = d3dClass->GetScreenSize();
            m_aspectRatio = viewSize.x / viewSize.y;
        }
        else
        {
            // Clear render target bundle
            m_renderTargetBundle->Clear(d3dClass->GetDeviceContext());
            viewSize = { static_cast<float>(m_renderTargetBundle->GetWidth()), static_cast<float>(m_renderTargetBundle->GetHeight()) };
        }

        CalculateProjectionMatrix();
        ViewConstants viewConstants;
        viewConstants.view = viewMatrix;
        viewConstants.projection = m_projectionMatrix;
        viewConstants.cameraPosition = sceneNode->GetWorldSpacePosition();
        viewConstants.clipInfo = GetClipInfo();
        viewConstants.projectionInfo = GetProjInfo(viewSize);
        viewConstants.invViewSize = Utils::Maths::Vector2(1.0f, 1.0f) / viewSize;
        viewConstants.projectionScale = GetProjectionScale(viewSize);
        viewConstants.jitteredProjection = JitterProjection(viewSize);

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

    // All taken from G3D engine. http://g3d.sourceforge.net/
    Utils::Maths::Vector3 Camera::GetClipInfo() const
    {
        return{ m_nearClip * m_farClip, m_nearClip - m_farClip, m_farClip };
    }

    
    Utils::Maths::Vector4 Camera::GetProjInfo(const Utils::Maths::Vector2& viewSize) const
    {
        return{ 2.0f / (viewSize.x * m_projectionMatrix._11),
                -2.0f / (viewSize.y * m_projectionMatrix._22),
                -(1.0f - m_projectionMatrix._13) / m_projectionMatrix._11,
                (1.0f + m_projectionMatrix._23) / m_projectionMatrix._22};
    }

    float Camera::GetProjectionScale(const Utils::Maths::Vector2& viewSize) const
    {
        const float scale = 2.0f * tan(m_fov * 0.5f);

        return viewSize.y / scale;
    }

    Utils::Maths::Matrix Camera::JitterProjection(const Utils::Maths::Vector2& viewSize)
    {
        if (m_jitterEnabled && !m_jitterSequence.empty())
        {
            auto currentJitter = m_jitterSequence[m_jitterCount];
            Utils::Maths::Vector3 tranlation = { ((currentJitter.x * 2.0f) -1.0f)/ viewSize.x, ((currentJitter.y * 2.0f) - 1.0f) / viewSize.y, 0.0f };

            auto jitter = Utils::Maths::Matrix::CreateFromTranslation(tranlation);

            m_jitterCount = (m_jitterCount+1) % m_jitterSequence.size();

            return m_projectionMatrix * jitter;
        }

        return m_projectionMatrix;
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
        // TODO: Camera parameters not changing. FIX
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
        // Min must be more than 0 otherwise projection matrix calculation fails due to DirectXMath assert.
        RegisterScalarProperty(L"Near Plane", nearPlaneGetter, nearPlaneSetter, 0.001f, 100.0f);

        auto farPlaneGetter = [this]() { return m_farClip; };
        auto farPlaneSetter = [this](float v) { m_farClip = v; };
        RegisterScalarProperty(L"Far Plane", farPlaneGetter, farPlaneSetter, 50.0f, 1000.0f);

        auto orthoSizeGetter = [this]() { return m_orthographicSize; };
        auto orthoSizeSetter = [this](float v) { m_orthographicSize = v; };
        RegisterScalarProperty(L"Orthographic Size", orthoSizeGetter, orthoSizeSetter, 1.0f, 1000.0f);

    }
}
