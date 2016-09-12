#pragma once

//////////////
// INCLUDES //
//////////////
#include "pch.h"

class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();
	
	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

    Utils::Maths::Matrix GetProjectionMatrix();
    Utils::Maths::Matrix GetWorldMatrix();
    Utils::Maths::Matrix GetOrthoMatrix();

	void GetVideoCardInfo(char*, int&);

	void TurnZBufferOn();
	void TurnZBufferOff();
	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	std::string m_videoCardDescription;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11Debug> m_debugDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
    Utils::Maths::Matrix m_projectionMatrix;
    Utils::Maths::Matrix m_worldMatrix;
    Utils::Maths::Matrix m_orthoMatrix;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaEnableBlendingState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaDisableBlendingState;
};
