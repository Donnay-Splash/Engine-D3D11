#pragma once
#include "pch.h"
#include <Scene\Scene.h>
#include <Scene\Components\Light.h>
#include <Resources\ConstantBuffer.h>
#include <Resources\ConstantBufferLayouts.h>

// Class for managing lights in the scene. e.g. Sorting and picking most relevant lights
// Managing light properties, Light probes, etc...
class LightManager
{
public:
    using Ptr = std::shared_ptr<LightManager>;
    LightManager();
    LightManager(const LightManager&) = delete;
    ~LightManager();

    void Initialize(ID3D11Device* device);
    // Gathers lights and uploads data to GPU
    void GatherLights(Scene::Ptr scene, ID3D11DeviceContext* deviceContext);

    static const uint32_t kMaxLightCount = 4;

private:
    ConstantBuffer<LightConstants>::Ptr m_lightBuffer;
};