#include "pch.h"
#include <Scene\Components\ShadowMapCamera.h>

namespace UM = Utils::Maths;

namespace Engine
{
    ShadowMapCamera::ShadowMapCamera(Component::SceneNodePtr sceneNode) : Camera(sceneNode)
    {

    }

    void ShadowMapCamera::Initialize()
    {
        // initialise base camera
        Camera::Initialize();

        m_lightViewConstants = std::make_shared<ConstantBuffer<ShadowMapConstants>>(PipelineStage::Vertex);

        // For now we are only dealing with shadows for directional lights
        SetProjectionMode(Camera::ProjectionMode::Orthographic);
        SetAspectRatio(1.0f);
        SetOrthographicSize(20.0f);
        SetNearClip(0.0f);
    }

    void ShadowMapCamera::Render(Scene::Ptr scene, Light::Ptr light, ShaderPipeline::Ptr shadowMapPipeline)
    {
        // We aren't rendering shadows to the backbuffer
        // so render target is required to be non null
        EngineAssert(m_renderTargetBundle != nullptr);
        EngineAssert(shadowMapPipeline != nullptr);

        // clear the target before rendering
        auto device = scene->GetDevice();
        auto deviceContext = device->GetDeviceContext();
        m_renderTargetBundle->Clear(deviceContext);
        device->SetRenderTarget(m_renderTargetBundle);

        // We want to place the light outside of the scene bounds so that it includes all visible objects
        auto lightNode = light->GetSceneNode();
        auto bounds = scene->GetSceneBounds();
        auto lightDirection = light->GetLightData().Direction;
        auto absDir = UM::Vector3::Abs(lightDirection);
        float dist = UM::Vector3::Dot(absDir, bounds.GetSize());
        auto position = bounds.GetCentre() + (-lightDirection * dist);
        lightNode->SetPosition(position, true);

        auto transform = light->GetSceneNode()->GetWorldTransform();
        auto up = UM::Vector3::Abs(transform.Up());
        auto right = UM::Vector3::Abs(transform.Right());

        float orthographicHeight = UM::Vector3::Dot(up, bounds.GetSize());
        float orthographicWidth = UM::Vector3::Dot(right, bounds.GetSize());

        float aspect = orthographicWidth / orthographicHeight;
        SetOrthographicSize(orthographicHeight * 2.0f);
        SetAspectRatio(aspect);
        SetFarClip(abs(dist) * 2.0f);

        auto worldToLightMatrix = lightNode->GetWorldTransform().GetInverse();

        CalculateProjectionMatrix();

        m_worldToProjectedLightSpace = worldToLightMatrix * m_projectionMatrix;

        ShadowMapConstants constants;
        constants.WorldToProjectedLightSpace = m_worldToProjectedLightSpace;
        m_lightViewConstants->SetData(constants);
        m_lightViewConstants->UploadData(deviceContext);

        scene->Render(shadowMapPipeline);
    }

}// end namespace Engine