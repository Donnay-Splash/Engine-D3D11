#pragma once

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "InputManager.h"
#include "d3dclass.h"
#include "Camera.h"
#include "timerclass.h"
#include "positionclass.h"
#include "Mesh.h"
#include "Light.h"
#include "ShaderManager.h"
#include "Scene.h"
#include "LightManager.h"
#include "EngineCreateOptions.h"

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
};

