#pragma once
#include "Component.h"
#include "ConstantBuffer.h"
#include "ConstantBufferLayouts.h"
#include "RenderTarget.h"
#include "DepthBuffer.h"
#include "d3dclass.h"
#include "Scene.h"

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
    ~Camera();

    void Render(ID3D11DeviceContext* deviceContext) const override;
    void Render(D3DClass::Ptr d3dClass, Scene::Ptr scene);
    void Update(float frameTime) override;

    // Sets the vertical field of view for the camera
    void SetFieldOfView(float fov) { m_fov = fov; }
    void SetProjectionMode(ProjectionMode projection) { m_projectionMode = projection; }
    void SetOrthographicSize(float othographicSize) { m_orthographicSize = othographicSize; }
    void SetAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; }

    void SetRenderTarget(RenderTarget::Ptr renderTarget);
    void SetDepthBuffer(DepthBuffer::Ptr renderTarget);

    float GetFieldOfView() const { return m_fov; }
    ProjectionMode GetProjectionMode() const { return m_projectionMode; }
    float GetOrthographicSize() const { return m_orthographicSize; }
    float GetAspectRatio() const { return m_aspectRatio; }

    RenderTarget::Ptr GetRenderTarget() const { return m_renderTarget; }
    DepthBuffer::Ptr GetDepthBuffer() const { return m_depthBuffer; }

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
    float m_aspectRatio;
    float m_orthographicSize;
    ProjectionMode m_projectionMode;
    
    // Note: Given an identity matrix the camera will be centred at the origin
    // looking along the positive z axis with up on the y axis
    Utils::Maths::Matrix m_projectionMatrix;

    friend class SceneNode;
};
