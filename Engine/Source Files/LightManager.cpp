#include "pch.h"
#include "LightManager.h"

static const uint32_t kEnvironmentMapRegister = 10;
static const uint32_t kShadowMapRegister = 11;

static const uint32_t kMaxShadowCasters = 2;
static const uint32_t kShadowMapWidth = 2048;
namespace Engine
{
    LightManager::LightManager()
    {
    }

    LightManager::~LightManager()
    {

    }

    void LightManager::Initialize(ID3D11Device* device, Scene::Ptr scene, ShaderPipeline::Ptr shadowMapPipeline)
    {
        m_lightBuffer = std::make_shared<ConstantBuffer<LightConstants>>(PipelineStage::Pixel, device);
        m_environmentSampler = std::make_shared<Sampler>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

        // Don't need to add any render targets as we are only rendering to depth
        m_shadowMapTarget = std::make_shared<RenderTargetBundle>(device, kShadowMapWidth, kShadowMapWidth, 1, 0, true);
        m_shadowMapTarget->Finalise();

        m_shadowMapPipeline = shadowMapPipeline;
        EngineAssert(m_shadowMapPipeline != nullptr);

        // Create the shadow mapping camera
        auto cameraNode = scene->AddNode();
        m_shadowMapCamera = cameraNode->AddComponent<ShadowMapCamera>();
        m_shadowMapCamera->SetFarClip(100.0f);
    }

    void LightManager::SetEnvironmentMap(Texture::Ptr envMap)
    {
        m_environmentMap = envMap;
    }

    void LightManager::GatherLights(Scene::Ptr scene, ID3D11DeviceContext* deviceContext, LightSpaceModifier space /*= LightSpaceModifier::World*/)
    {
        auto lights = scene->GetAllComponentsOfType<Light>();
        uint32_t lightCount = std::min(kMaxLightCount, static_cast<uint32_t>(lights.size()));
        LightConstants lightBuffer;

        Light::Ptr shadowCaster;
        // Collect the data for all lights in the scene
        for (uint32_t i = 0; i < lightCount; i++)
        {
            lightBuffer.lights[i] = lights[i]->GetLightData();
            float intensity = lights[i]->GetIntensity();
            lightBuffer.lights[i].Color *= intensity;
            if (space == LightSpaceModifier::Camera)
            {
                // Lighting is performed in camera space so we want to transform the directions and positions to camera space
                auto cameraTransform = scene->GetWorldToCameraTransform();
                lightBuffer.lights[i].Transform(cameraTransform);
            }

            if (lights[i]->CastsShadows() && shadowCaster == nullptr)
            {
                shadowCaster = lights[i];
            }
        }
        lightBuffer.activeLights = static_cast<float>(lightCount);

        if (m_environmentMap != nullptr)
        {
            lightBuffer.envMapMipLevels = static_cast<float>(m_environmentMap->GetMipLevels());
            m_environmentMap->UploadData(deviceContext, PipelineStage::Pixel, kEnvironmentMapRegister);
            lightBuffer.envMapEnabled = 1.0f;
        }
        // Render and submit shadow map data
        if (shadowCaster != nullptr)
        {
            lightBuffer.shadowMapEnabled = 1.0f;
            // Concatenate matrices to convert from camera to projected light space
            lightBuffer.shadowMapTransform = RenderShadowMap(shadowCaster, deviceContext);
            lightBuffer.shadowMapDimensions = { float(kShadowMapWidth), float(kShadowMapWidth) };
        }

        m_environmentSampler->UploadData(deviceContext, kEnvironmentMapRegister);
        m_lightBuffer->SetData(lightBuffer);
        m_lightBuffer->UploadData(deviceContext);
    }

    Utils::Maths::Matrix LightManager::RenderShadowMap(Light::Ptr shadowCaster, ID3D11DeviceContext* deviceContext)
    {
        // No need to render shadows if there are no shadow casters
        EngineAssert(shadowCaster != nullptr);

        auto scene = shadowCaster->GetSceneNode()->GetScene();
        
        auto device = scene->GetDevice();
        device->BeginRenderEvent(L"Render Shadow maps");
            m_shadowMapCamera->SetRenderTargetBundle(m_shadowMapTarget);
            m_shadowMapCamera->Render(scene, shadowCaster, m_shadowMapPipeline);
            // Upload m_shadowMapTargets to shader
            m_shadowMapTarget->SetShaderResources(deviceContext, kShadowMapRegister);
        device->EndRenderEvent();

        // Calculate camera to light space transform
        auto worldToLight = m_shadowMapCamera->GetWorldToProjectedLightSpaceTransform();
        auto cameraToWorld = scene->GetCameraTransform();
        return cameraToWorld * worldToLight;
    }
}