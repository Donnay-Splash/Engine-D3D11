#pragma once

//////////////
// INCLUDES //
//////////////
#include "pch.h"
#include "RenderTarget.h"
#include "DepthBuffer.h"

class D3DClass
{
public:
    D3DClass();
    D3DClass(const D3DClass&);
    ~D3DClass();

    void Initialize(int, int, bool, HWND, bool, float, float);
    void Shutdown();
    
    void BeginScene(float, float, float, float);
    void EndScene();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();

    void GetVideoCardInfo(char*, int&);

    void TurnZBufferOn();
    void TurnZBufferOff();
    void TurnOnAlphaBlending();
    void TurnOffAlphaBlending();

private:
    void GetAdapterInformation();
    void CreateDeviceAndSwapChain(int screenWidth, int screenHeight, HWND hwnd);

private:
    bool m_vsync_enabled;
    int m_videoCardMemory;
    std::string m_videoCardDescription;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11Device1> m_device;
    Microsoft::WRL::ComPtr<ID3D11Debug> m_debugDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
    Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaEnableBlendingState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaDisableBlendingState;
    RenderTarget::Ptr m_backBufferRT;
    DepthBuffer::Ptr m_depthBuffer;
};
