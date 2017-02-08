#pragma once


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "InputManager.h"
#include "d3dclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "ShaderManager.h"
#include "LightManager.h"
#include "EngineCreateOptions.h"
#include <Scene\Components\PostProcessingCamera.h>
#include <Scene\Components\Light.h>
#include <Scene\Scene.h>
#include <Resources\Mesh.h>
#include <Resources\Sampler.h>

// Don't really want this sat here.
#include <Resources\ConstantBufferLayouts.h>

namespace Engine
{
    class Engine
    {
    public:
        using Ptr = std::shared_ptr<Engine>;
        Engine();
        ~Engine();

        bool Initialize(EngineCreateOptions createOptions);
        bool Frame();

        void ResizeBuffers(uint32_t newWidth, uint32_t newHeight);

        void SetFrameInput(InputState newInputState);

        InputManager::Ptr GetInputManager() const { return m_inputManager; }

        void LoadFile(const uint8_t* data, uint64_t byteCount);

    private:
        bool HandleInput(float);
        bool RenderGraphics();
        void InitializeScene();

    private:
        InputManager::Ptr m_inputManager;
        std::shared_ptr<D3DClass> m_direct3D;
        std::shared_ptr<Light> m_light1;
        std::shared_ptr<TimerClass> m_timer;
        std::shared_ptr<PositionClass> m_position;
        Mesh::Ptr m_mesh;
        Camera::Ptr m_camera;
        ShaderManager::Ptr m_shaderManager;
        Scene::Ptr m_scene;
        LightManager m_lightManager;
        EngineCreateOptions m_createOptions;

        // These can be moved to a more specific application class
        PostProcessingCamera::Ptr m_postProcessCamera;
        PostEffect<PostEffectConstants>::Ptr m_postEffect;
        Texture::Ptr m_prevDepth;
        Sampler::Ptr m_depthSampler;

    };
}

