#pragma once
#include <Scene\Scene.h>
#include <Scene\Components\Light.h>
#include <Resources\ConstantBuffer.h>
#include <Resources\ConstantBufferLayouts.h>
#include <Resources\Texture.h>
#include <Resources\Sampler.h>

namespace Engine
{
    /*  Controls what space the light data is in when sent to the shader*/
    enum class LightSpaceModifier
    {
        World,
        Camera
    };

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
        void GatherLights(Scene::Ptr scene, ID3D11DeviceContext* deviceContext, LightSpaceModifier space = LightSpaceModifier::World);
        void SetEnvironmentMap(Texture::Ptr envMap);

        static const uint32_t kMaxLightCount = 4;

    private:
        ConstantBuffer<LightConstants>::Ptr m_lightBuffer;
        Texture::Ptr m_environmentMap;
        Sampler::Ptr m_environmentSampler;
    };
}