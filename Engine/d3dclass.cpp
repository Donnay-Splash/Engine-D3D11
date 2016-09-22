////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "d3dclass.h"


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


void D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, 
                          float screenDepth, float screenNear)
{
    ID3D11Texture2D* backBufferPtr;
    D3D11_VIEWPORT viewport;

    // Store the vsync setting.
    m_vsync_enabled = vsync;

    m_screenSize = { static_cast<float>(screenWidth), static_cast<float>(screenHeight) };

    // Create a DirectX graphics interface factory.
    Utils::DirectXHelpers::ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(m_factory.GetAddressOf())));

    // Retrieve information from the adapter.
    GetAdapterInformation();

    // Create the device and swap chain
    CreateDeviceAndSwapChain(screenWidth, screenHeight, hwnd);

    // Get the pointer to the back buffer.
    Utils::DirectXHelpers::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferPtr)));

    // Create the render target view with the back buffer pointer.
    m_backBufferRT = std::make_shared<RenderTarget>(backBufferPtr, 0, m_device.Get());

    // Release pointer to the back buffer as we no longer need it.
    backBufferPtr->Release();
    backBufferPtr = nullptr;

    m_depthBuffer = std::make_shared<DepthBuffer>(screenWidth, screenHeight, 0, m_device.Get());

    // Setup the viewport for rendering.
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // Create the viewport.
    m_deviceContext->RSSetViewports(1, &viewport);
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

void D3DClass::CreateDeviceAndSwapChain(int screenWidth, int screenHeight, HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // Set to a single back buffer.
    swapChainDesc.BufferCount = 1;
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

    // Create the swap chain
    auto hr = m_factory->CreateSwapChainForHwnd(m_device.Get(), hwnd, &swapChainDesc, NULL, NULL, m_swapChain.GetAddressOf());
    Utils::DirectXHelpers::ThrowIfFailed(hr);
}


void D3DClass::Shutdown()
{
    // Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
    if(m_swapChain)
    {
        m_swapChain->SetFullscreenState(false, NULL);
    }

    m_deviceContext = nullptr;
    m_device = nullptr;
    m_swapChain = nullptr;
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
    m_deviceContext->ClearDepthStencilView(m_depthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}


void D3DClass::EndScene()
{
    // Present the back buffer to the screen since rendering is complete.
    if(m_vsync_enabled)
    {
        // Lock to screen refresh rate.
        m_swapChain->Present(1, 0);
    }
    else
    {
        // Present as fast as possible.
        m_swapChain->Present(0, 0);
    }

    return;
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

void D3DClass::SetRenderTarget(RenderTarget::Ptr renderTarget, DepthBuffer::Ptr depthBuffer) const
{
    if (renderTarget == nullptr)
    {
        renderTarget = m_backBufferRT;
    }
    if (depthBuffer == nullptr)
    {
        depthBuffer = m_depthBuffer;
    }

    m_deviceContext->OMSetRenderTargets(1, renderTarget->GetRTV().GetAddressOf(), depthBuffer->GetDSV().Get());
}