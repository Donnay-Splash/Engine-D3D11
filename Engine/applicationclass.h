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
#include "textureshaderclass.h"
#include "timerclass.h"
#include "positionclass.h"
#include "Mesh.h"
#include "vmshaderclass.h"
#include "lightclass.h"
#include "planeclass.h"
#include "ShaderManager.h"


class ApplicationClass
{
public:
	ApplicationClass();
	~ApplicationClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool HandleInput(float);
	bool RenderGraphics();

private:
	std::shared_ptr<InputClass> m_Input;
	std::shared_ptr<D3DClass> m_Direct3D;
	std::shared_ptr<CameraClass> m_Camera;
	std::shared_ptr<VMShaderClass> m_VMShader;
	std::shared_ptr<LightClass> m_Light1;
	std::shared_ptr<TimerClass> m_Timer;
	std::shared_ptr<PositionClass> m_Position;
	std::shared_ptr<Mesh> m_Mesh;
	ShaderManager::Ptr m_shaderManager;
};

