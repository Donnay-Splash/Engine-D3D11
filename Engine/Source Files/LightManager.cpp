#include "pch.h"
#include "LightManager.h"

static const uint32_t kEnvironmentMapRegister = 10;
namespace Engine
{
    LightManager::LightManager()
    {

    }

    LightManager::~LightManager()
    {

    }

    void LightManager::Initialize(ID3D11Device* device)
    {
        m_lightBuffer = std::make_shared<ConstantBuffer<LightConstants>>(PipelineStage::Pixel, device);
        m_environmentSampler = std::make_shared<Sampler>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
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
        }
        lightBuffer.activeLights = static_cast<float>(lightCount);

        if (m_environmentMap != nullptr)
        {
            lightBuffer.envMapMipLevels = static_cast<float>(m_environmentMap->GetMipLevels());
            m_environmentMap->UploadData(deviceContext, PipelineStage::Pixel, kEnvironmentMapRegister);
            lightBuffer.envMapEnabled = 1.0f;
        }
        m_environmentSampler->UploadData(deviceContext, kEnvironmentMapRegister);
        m_lightBuffer->SetData(lightBuffer);
        m_lightBuffer->UploadData(deviceContext);
    }
}