#include "pch.h"
#include "LightManager.h"

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

        m_lightBuffer->SetData(lightBuffer);
        m_lightBuffer->UploadData(deviceContext);
    }
}