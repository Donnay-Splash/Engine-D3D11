#pragma once
#include "Component.h"
#include "d3dclass.h"
#include <Resources\ConstantBuffer.h>
#include <Resources\ConstantBufferLayouts.h>
#include <Resources\RenderTarget.h>
#include <Resources\DepthBuffer.h>
#include <Scene\Scene.h>

namespace Engine
{
    class Camera : public Component
    {
    public:
        enum class ProjectionMode
        {
            Perspective,
            Orthographic
        };

    public:
        using Ptr = std::shared_ptr<Camera>;
        Camera(const Camera&) = delete;
        virtual ~Camera();

        void Render(ID3D12GraphicsCommandList* commandList, Scene::Ptr scene);

        // Sets the vertical field of view for the camera
        void SetFieldOfView(float fov) { m_fov = fov; }
        void SetProjectionMode(ProjectionMode projection) { m_projectionMode = projection; }
        void SetOrthographicSize(float othographicSize) { m_orthographicSize = othographicSize; }
        void SetAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; }
        void SetJitterEnabled(bool enabled) { m_jitterEnabled = enabled; }
        void SetJitterSequence(std::vector<Utils::Maths::Vector2> sequence) { m_jitterSequence = sequence; }
        void SetFarClip(float farClip) { m_farClip = std::max(farClip, m_nearClip); }
        void SetNearClip(float nearClip) { m_nearClip = std::min(nearClip, m_farClip); }

        void SetRenderTargetBundle(RenderTargetBundle::Ptr renderTargetBundle);

        float GetFieldOfView() const { return m_fov; }
        ProjectionMode GetProjectionMode() const { return m_projectionMode; }
        float GetOrthographicSize() const { return m_orthographicSize; }
        float GetAspectRatio() const { return m_aspectRatio; }
        bool GetJitterEnabled() const { return m_jitterEnabled; }
        Utils::Maths::Vector2 GetCurrentTemporalJitter() const { return m_currentJitter; }

        RenderTargetBundle::Ptr GetRenderTargetBundle() const { return m_renderTargetBundle; }

    protected:
        Camera(Component::SceneNodePtr sceneNode);
        virtual void Initialize() override;
        virtual void CalculateProjectionMatrix();
        // Camera info required for reconstructing camera space Z from Depth 
        Utils::Maths::Vector3 GetClipInfo() const;
        // Camera info required for reconstructing camera space position 
        // from screen space coords and camera space z
        Utils::Maths::Vector4 GetProjInfo(const Utils::Maths::Vector2& viewSize) const;

        // Returns the number of pixels per metre at z = 1 for the given viewport.
        // This can be used to perform projections from world space values to screen space.
        // e.g. circle of confusion, AO sample radius
        float GetProjectionScale(const Utils::Maths::Vector2& viewSize) const;

        Utils::Maths::Matrix JitterProjection(const Utils::Maths::Vector2& viewSize);

        Utils::Maths::Matrix GetCameraToScreenSpaceMatrix(const Utils::Maths::Vector2& viewSize) const;

    protected:
        RenderTargetBundle::Ptr m_renderTargetBundle;
        // Note: Given an identity matrix the camera will be centred at the origin
        // looking along the positive z axis with up on the y axis
        Utils::Maths::Matrix m_projectionMatrix;
        
    private:
        void RegisterPublicProperties();

    private:
        ConstantBuffer<ViewConstants>::Ptr m_viewConstants;

        float m_fov;
        float m_nearClip;
        float m_farClip;
        float m_aspectRatio;
        float m_orthographicSize;
        ProjectionMode m_projectionMode;

        // When enabled, randomly jitters projection matrix on sub pixel level.
        // This is used for temporal super-sampling effects.
        // Jittered and non-jittered matrices are passed in order to calculate
        // accurate screen-space velocity vectors
        bool m_jitterEnabled = false;
        uint32_t m_jitterCount = 0;
        std::vector<Utils::Maths::Vector2> m_jitterSequence;
        Utils::Maths::Vector2 m_currentJitter;


        friend class SceneNode;
    };
}
