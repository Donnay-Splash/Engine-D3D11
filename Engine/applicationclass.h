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
#include "inputclass.h"
#include "d3dclass.h"
#include "cameraclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "Mesh.h"
#include "vmshaderclass.h"
#include "lightclass.h"
#include "ShaderManager.h"
#include "Scene.h"


class ApplicationClass
{
public:
    ApplicationClass();
    ~ApplicationClass();

    bool Initialize(HINSTANCE, HWND, int, int);
    bool Frame();

private:
    bool HandleInput(float);
    bool RenderGraphics();

private:
    std::shared_ptr<InputClass> m_input;
    std::shared_ptr<D3DClass> m_direct3D;
    std::shared_ptr<CameraClass> m_camera;
    std::shared_ptr<VMShaderClass> m_vmShader;
    std::shared_ptr<LightClass> m_light1;
    std::shared_ptr<TimerClass> m_timer;
    std::shared_ptr<PositionClass> m_position;
    std::shared_ptr<Mesh> m_mesh;
    ShaderManager::Ptr m_shaderManager;
    Scene::Ptr m_scene;
};

