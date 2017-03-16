////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "d3dclass.h"
#include "Engine.h"

namespace Engine
{
    D3DClass::D3DClass()
    {
        m_swapChain = nullptr;
        m_device = nullptr;
        m_deviceContext = nullptr;
    }


    D3DClass::D3DClass(const D3DClass& other)
    {
    }


    D3DClass::~D3DClass()
    {
        m_debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_FLAGS::D3D11_RLDO_DETAIL);
    }


    void D3DClass::Initialize(EngineCreateOptions createOptions)
    {
        // Store the vsync setting.
        m_createOptions = createOptions;
        m_vsync_enabled = m_createOptions.VSyncEnabled;

        EngineAssert(m_createOptions.ScreenWidth > 0);
        EngineAssert(m_createOptions.ScreenHeight > 0);
        m_screenSize = { static_cast<float>(m_createOptions.ScreenWidth), static_cast<float>(m_createOptions.ScreenHeight) };

        // Create a DirectX graphics interface factory.
        Utils::DirectXHelpers::ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(m_factory.GetAddressOf())));

        // Retrieve information from the adapter.
        GetAdapterInformation();

        // Create the device and swap chain
        CreateDeviceAndSwapChain();
    }

    void D3DClass::GetAdapterInformation()
    {
        Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
        DXGI_ADAPTER_DESC adapterDesc;

        // Use the factory to create an adapter for the primary graphics interface (video card).
        Utils::DirectXHelpers::ThrowIfFailed(m_factory->EnumAdapters(0, m_adapter.GetAddressOf()));

        // Get the adapter (video card) description.
        Utils::DirectXHelpers::ThrowIfFailed(m_adapter->GetDesc(&adapterDesc));

        // Store the dedicated video card memory in megabytes.
        m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

        // Convert the name of the video card from wide to standard character length.
        std::wstring wideAdapterDescription(adapterDesc.Description);
        m_videoCardDescription = std::string(wideAdapterDescription.begin(), wideAdapterDescription.end());
    }

    void D3DClass::CreateDeviceAndSwapChain()
    {
        // Set the feature level to DirectX 11.
        D3D_FEATURE_LEVEL featureLevelsSupported[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;
        Microsoft::WRL::ComPtr<ID3D11Device> tempDevice;
        Utils::DirectXHelpers::ThrowIfFailed(D3D11CreateDevice(m_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, creationFlags, featureLevelsSupported, ARRAYSIZE(featureLevelsSupported), D3D11_SDK_VERSION,
            tempDevice.GetAddressOf(), NULL, m_deviceContext.GetAddressOf()));
        // Now convert the device to a ID3D11Device1
        Utils::DirectXHelpers::ThrowIfFailed(tempDevice.As(&m_device));
        // Remove the temp device
        tempDevice = nullptr;
        // Query the debug device from the device
        Utils::DirectXHelpers::ThrowIfFailed(m_device.As(&m_debugDevice));
        // Query the UserDefinedAnnotation interface from the device context
        // This can fail when using Windows 7 so we don't want to throw
        // We will just fail to write events. No biggy.
        auto hr = m_deviceContext.As(&m_userDefinedAnnotation);
        if (hr != S_OK)
        {
            m_userDefinedAnnotation = nullptr;
        }

        switch (m_createOptions.RendererMode)
        {
        case EngineRendererMode::Win32:
            CreateSwapChain_HWND(m_createOptions.ScreenWidth, m_createOptions.ScreenHeight);
            break;
        case EngineRendererMode::XAML:
            CreateSwapChain_XAML(m_createOptions.ScreenWidth, m_createOptions.ScreenHeight);
            break;
        default:
            // Incorrect enum value passed in
            EngineAssert(false);
            break;
        }

        // Now call swapchain created callback.
        if (m_createOptions.SwapChainCreatedCallback != nullptr)
        {
            m_createOptions.SwapChainCreatedCallback(m_swapChain.Get(), m_createOptions.UserData);
        }
    }

    void D3DClass::CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight)
    {
        if (m_swapChain == nullptr)
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
            SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

            // Set to a single back buffer.
            swapChainDesc.BufferCount = kBufferCount;
            swapChainDesc.Width = screenWidth;
            swapChainDesc.Height = screenHeight;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

            // Set the usage of the back buffer.
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

            // Turn multisampling off.
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;

            // Discard the back buffer contents after presenting.
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            // Don't set the advanced flags.
            swapChainDesc.Flags = 0;

            // Create the swap chain
            auto hr = m_factory->CreateSwapChainForHwnd(m_device.Get(), m_createOptions.HWND, &swapChainDesc, NULL, NULL, m_swapChain.GetAddressOf());
            Utils::DirectXHelpers::ThrowIfFailed(hr);
        }
        else
        {
            // Resize swap chain
            // TODO: Need to remember to release back buffers.
            m_swapChain->ResizeBuffers(kBufferCount, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        }

        ID3D11Texture2D* backBufferPtr = nullptr;
        Utils::DirectXHelpers::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferPtr)));
        CreateBackBufferResources(backBufferPtr);
        backBufferPtr->Release();
        backBufferPtr = nullptr;
    }

    void D3DClass::CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight)
    {
        if (m_swapChain == nullptr)
        {
            //DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
            //SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

            //// Set to a single back buffer.
            //swapChainDesc.BufferCount = kBufferCount;
            //swapChainDesc.Width = screenWidth;
            //swapChainDesc.Height = screenHeight;
            //swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

            //// Set the usage of the back buffer.
            //swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

            //// Turn multisampling off.
            //swapChainDesc.SampleDesc.Count = 1;
            //swapChainDesc.SampleDesc.Quality = 0;

            //// Discard the back buffer contents after presenting.
            //swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

            //// Don't set the advanced flags.
            //swapChainDesc.Flags = 0;

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

            swapChainDesc.Width = lround(screenWidth);		// Match the size of the window.
            swapChainDesc.Height = lround(screenHeight);
            swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// This is the most common swap chain format.
            swapChainDesc.Stereo = false;
            swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Windows Store apps must use _FLIP_ SwapEffects.
            swapChainDesc.Flags = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

            // Create the swap chain
            auto hr = m_factory->CreateSwapChainForComposition(m_device.Get(), &swapChainDesc, nullptr, m_swapChain.GetAddressOf());
            Utils::DirectXHelpers::ThrowIfFailed(hr);
        }
        else
        {
            // Resize swap chain
            // TODO: Need to remember to release back buffers.
            m_swapChain->ResizeBuffers(kBufferCount, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        }

        ID3D11Texture2D* backBufferPtr = nullptr;
        Utils::DirectXHelpers::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferPtr)));
        CreateBackBufferResources(backBufferPtr);
        backBufferPtr->Release();
        backBufferPtr = nullptr;
    }

    void D3DClass::CreateBackBufferResources(ID3D11Texture2D* backBufferPtr)
    {
        // Create the render target view with the back buffer pointer.
        m_backBufferRT = std::make_shared<RenderTarget>(backBufferPtr, 0, m_device.Get());

        auto backBufferWidth = m_backBufferRT->GetWidth();
        auto backBufferHeight = m_backBufferRT->GetHeight();

        m_depthBuffer = std::make_shared<DepthBuffer>(backBufferWidth, backBufferHeight, 1, 0, m_device.Get());

        // Setup the viewport for rendering.
        D3D11_VIEWPORT viewport;
        viewport.Width = (float)backBufferWidth;
        viewport.Height = (float)backBufferHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

        // Create the viewport.
        m_deviceContext->RSSetViewports(1, &viewport);
    }

    void D3DClass::Shutdown()
    {
        // Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
        if (m_swapChain)
        {
            m_swapChain->SetFullscreenState(false, NULL);
        }

        m_deviceContext = nullptr;
        m_device = nullptr;
        m_swapChain = nullptr;
    }

    void D3DClass::ResizeBuffers(uint32_t newWidth, uint32_t newHeight)
    {
        if (newWidth != m_backBufferRT->GetWidth() || newHeight != m_backBufferRT->GetHeight())
        {
            ClearResources();
            // Resize buffers
            switch (m_createOptions.RendererMode)
            {
            case EngineRendererMode::Win32:
                CreateSwapChain_HWND(newWidth, newHeight);
                break;
            case EngineRendererMode::XAML:
                CreateSwapChain_XAML(newWidth, newHeight);
                break;
            default:
                // Incorrect enum value passed in
                EngineAssert(false);
                break;
            }

        }
        // otherwise continue as normal
    }


    void D3DClass::BeginScene(float red, float green, float blue, float alpha)
    {
        float color[4];


        // Setup the color to clear the buffer to.
        color[0] = red;
        color[1] = green;
        color[2] = blue;
        color[3] = alpha;

        // Clear the back buffer.
        m_deviceContext->ClearRenderTargetView(m_backBufferRT->GetRTV().Get(), color);

        // Clear the depth buffer.
        m_deviceContext->ClearDepthStencilView(m_depthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        return;
    }


    void D3DClass::EndScene()
    {
        // Present the back buffer to the screen since rendering is complete.
        if (m_vsync_enabled)
        {
            // Lock to screen refresh rate.
            m_swapChain->Present(1, 0);
        }
        else
        {
            // Present as fast as possible.
            m_swapChain->Present(0, 0);
        }

        // Clear pixel and vertex shader texture slots
        // TODO: Likely unnecessary to clear all slots. Look at estimating max texture count
        ID3D11ShaderResourceView* nullViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
        m_deviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullViews);
        m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullViews);
        UnbindAllRenderTargets();

        return;
    }

    void D3DClass::UnbindAllRenderTargets() const
    {
        // Clear render target state
        ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        m_deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullViews, nullptr);
    }


    ID3D11Device* D3DClass::GetDevice()
    {
        return m_device.Get();
    }


    ID3D11DeviceContext* D3DClass::GetDeviceContext()
    {
        return m_deviceContext.Get();
    }


    void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
    {
        strcpy_s(cardName, 128, m_videoCardDescription.c_str());
        memory = m_videoCardMemory;
        return;
    }

    Utils::Maths::Vector2 D3DClass::GetScreenSize() const
    {
        EngineAssert(m_backBufferRT != nullptr);

        return Utils::Maths::Vector2(static_cast<float>(m_backBufferRT->GetWidth()), static_cast<float>(m_backBufferRT->GetHeight()));
    }

    void D3DClass::SetRenderTarget(RenderTargetBundle::Ptr bundle) const
    {
        if (bundle == nullptr)
        {
            m_deviceContext->OMSetRenderTargets(1, m_backBufferRT->GetRTV().GetAddressOf(), m_depthBuffer->GetDSV().Get());
        }
        else
        {
            ID3D11DepthStencilView* dsv = nullptr;
            if (bundle->GetDepthBuffer() != nullptr) dsv = bundle->GetDepthBuffer()->GetDSV().Get();
            m_deviceContext->OMSetRenderTargets(bundle->RenderTargetCount(), bundle->GetRenderTargetViews(), dsv);
        }
    }

    void D3DClass::ClearResources()
    {
        m_backBufferRT = nullptr;
        m_depthBuffer = nullptr;

        m_deviceContext->Flush();
    }

    Texture::Ptr D3DClass::CopyTexture(Texture::Ptr textureToCopy) const
    {
        EngineAssert(textureToCopy != nullptr);
        auto newTexture = Texture::CreateIdenticalTexture(textureToCopy, m_device.Get());

        m_deviceContext->CopyResource(newTexture->GetTexture().Get(), textureToCopy->GetTexture().Get());

        return newTexture;
    }

    Texture::Ptr D3DClass::CopyBackBuffer() const
    {
        return CopyTexture(m_backBufferRT->GetTexture());
    }

    void D3DClass::BeginRenderEvent(const std::wstring& eventLabel)
    {
        if (m_userDefinedAnnotation != nullptr)
        {
            m_userDefinedAnnotation->BeginEvent(eventLabel.c_str());
        }
    }

    void D3DClass::EndRenderEvent()
    {
        if (m_userDefinedAnnotation != nullptr)
        {
            m_userDefinedAnnotation->EndEvent();
        }
    }
}