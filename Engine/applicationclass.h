////////////////////////////////////////////////////////////////////////////////
// Filename: applicationclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_


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
#include "meshclass.h"
#include "vmshaderclass.h"
#include "lightclass.h"
#include "planeclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ApplicationClass
////////////////////////////////////////////////////////////////////////////////
class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

private:
	bool HandleInput(float);
	bool RenderGraphics();

private:
	float rotation;
	float plane_height;
	float plane_length;
	std::shared_ptr<InputClass> m_Input;
	std::shared_ptr<D3DClass> m_Direct3D;
	std::shared_ptr<CameraClass> m_Camera;
	std::shared_ptr<VMShaderClass> m_VMShader;
	std::shared_ptr<LightClass> m_Light1;
	std::shared_ptr<TimerClass> m_Timer;
	std::shared_ptr<PositionClass> m_Position;
	std::shared_ptr<MeshClass> m_Mesh;
	std::shared_ptr<PlaneClass> m_Plane;
};

#endif