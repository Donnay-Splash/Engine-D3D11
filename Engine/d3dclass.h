#pragma once

//////////////
// INCLUDES //
//////////////
#include "pch.h"
#include "RenderTarget.h"
#include "DepthBuffer.h"
#include "EngineCreateOptions.h"

// Forward Declarations
struct EngineCreateOptions;

class D3DClass
{
public:
    using Ptr = std::shared_ptr<D3DClass>;
    D3DClass();
    D3DClass(const D3DClass&);
    ~D3DClass();

    void Initialize(EngineCreateOptions createOptions, float screenDepth, float screenNear);
    void Shutdown();
    
    void ResizeBuffers(uint32_t newWidth, uint32_t newHeight);
    
    void BeginScene(float, float, float, float);
    void EndScene();

    void SetRenderTarget(RenderTarget::Ptr renderTarget, DepthBuffer::Ptr depthBuffer) const;

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();

    void GetVideoCardInfo(char*, int&);

    Utils::Maths::Vector2 GetScreenSize() const { return m_screenSize; }

private:
    void GetAdapterInformation();
    void CreateDeviceAndSwapChain();
    void CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight);
    void CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight);

    void CreateBackBufferResources(ID3D11Texture2D* backbufferPtr);

    void ClearResources();

private:
    bool m_vsync_enabled;
    int m_videoCardMemory;
    std::string m_videoCardDescription;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11Device1> m_device;
    Microsoft::WRL::ComPtr<ID3D11Debug> m_debugDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;
    RenderTarget::Ptr m_backBufferRT;
    DepthBuffer::Ptr m_depthBuffer;
    Utils::Maths::Vector2 m_screenSize;

    // Use a single back buffer for now.
    static const UINT kBufferCount = 1;
    EngineCreateOptions m_createOptions;
};
