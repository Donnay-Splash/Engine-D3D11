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

        void Render(ID3D11DeviceContext* deviceContext) const override;
        void Render(D3DClass::Ptr d3dClass, Scene::Ptr scene);
        void Update(float frameTime) override;

        // Sets the vertical field of view for the camera
        void SetFieldOfView(float fov) { m_fov = fov; }
        void SetProjectionMode(ProjectionMode projection) { m_projectionMode = projection; }
        void SetOrthographicSize(float othographicSize) { m_orthographicSize = othographicSize; }
        void SetAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; }
        void SetJitterEnabled(bool enabled) { m_jitterEnabled = enabled; }
        void SetJitterSequence(std::vector<Utils::Maths::Vector2> sequence) { m_jitterSequence = sequence; }

        void SetRenderTargetBundle(RenderTargetBundle::Ptr renderTargetBundle);

        float GetFieldOfView() const { return m_fov; }
        ProjectionMode GetProjectionMode() const { return m_projectionMode; }
        float GetOrthographicSize() const { return m_orthographicSize; }
        float GetAspectRatio() const { return m_aspectRatio; }
        bool GetJitterEnabled() const { return m_jitterEnabled; }

        RenderTargetBundle::Ptr GetRenderTargetBundle() const { return m_renderTargetBundle; }

    protected:
        Camera(Component::SceneNodePtr sceneNode);
        virtual void Initialize(ID3D11Device* device) override;
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

    protected:
        RenderTargetBundle::Ptr m_renderTargetBundle;
        
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

        // Note: Given an identity matrix the camera will be centred at the origin
        // looking along the positive z axis with up on the y axis
        Utils::Maths::Matrix m_projectionMatrix;

        friend class SceneNode;
    };
}
