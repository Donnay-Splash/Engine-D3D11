#pragma once

//////////////
// INCLUDES //
//////////////
#include "EngineCreateOptions.h"
#include <RenderTargetBundle.h>

namespace Engine
{
    // Forward Declarations
    struct EngineCreateOptions;

    class D3DClass
    {
    public:
        using Ptr = std::shared_ptr<D3DClass>;
        D3DClass();
        D3DClass(const D3DClass&);
        ~D3DClass();

        void Initialize(EngineCreateOptions createOptions);
        void Shutdown();

        void ResizeBuffers(uint32_t newWidth, uint32_t newHeight);

        void BeginScene(float, float, float, float);
        void EndScene();

        // Potentially rename FrameBuffer?
        void SetRenderTarget(RenderTargetBundle::Ptr) const;

        ID3D11Device* GetDevice();
        ID3D11DeviceContext* GetDeviceContext();

        void GetVideoCardInfo(char*, int&);

        Utils::Maths::Vector2 GetScreenSize() const;

        Texture::Ptr CopyTexture(Texture::Ptr textureToCopy) const;

        Texture::Ptr CopyBackBuffer() const;

        void UnbindAllRenderTargets() const;

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
        static const UINT kBufferCount = 2;
        EngineCreateOptions m_createOptions;
    };
}
