#pragma once
#include "Component.h"
#include "ConstantBuffer.h"
#include "ConstantBufferLayouts.h"
#include "RenderTarget.h"
#include "DepthBuffer.h"

class Camera : public Component
{
public:
    enum class ProjectionMode
    {
        Perspective,
        Orthographic
    };

public:
    Camera(const Camera&) = delete;
    ~Camera();

    void Render(ID3D11DeviceContext* deviceContext) const override;
    void Update(float frameTime) override;

    // Sets the vertical field of view for the camera
    void SetFieldOfView(float fov) { m_fov = fov; }
    void SetProjectionMode(ProjectionMode projection) { m_projectionMode = projection; }

    void SetRenderTarget(RenderTarget::Ptr renderTarget);
    void SetDepthBuffer(DepthBuffer::Ptr renderTarget);

protected:
    Camera(Component::SceneNodePtr sceneNode); 
    virtual void Initialize(ID3D11Device* device) override;
    virtual void CalculateProjectionMatrix();

private:
    ConstantBuffer<ViewConstants>::Ptr m_viewConstants;
    RenderTarget::Ptr m_renderTarget;
    DepthBuffer::Ptr m_depthBuffer;

    float m_fov;
    float m_nearClip;
    float m_farClip;
    ProjectionMode m_projectionMode;
    
    // Note: Given an identity matrix the camera will be centred at the origin
    // looking down the positive z axis with up on the y axis
    Utils::Maths::Matrix m_projectionMatrix;
};
