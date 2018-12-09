////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "d3dclass.h"
#include "DX11Impl.h"

namespace Engine
{
	static D3DClass* s_Instance = nullptr;
	void D3DClass::Initialize(EngineCreateOptions createOptions)
	{
		// TODO: Add platform to create options so that we can dynamically switch
		EngineAssert(s_Instance == nullptr);
		s_Instance = new DX11Impl();
		s_Instance->Initialize_Internal(createOptions);
	}

	D3DClass* D3DClass::Instance()
	{
		EngineAssert(s_Instance != nullptr);
		return s_Instance;
	}
}