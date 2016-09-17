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
    m_depthStencilBuffer = nullptr;
    m_depthStencilState = nullptr;
    m_rasterState = nullptr;
    m_depthDisabledStencilState = nullptr;
    m_alphaEnableBlendingState = nullptr;
    m_alphaDisableBlendingState = nullptr;
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
    D3D11_RASTERIZER_DESC rasterDesc;
    D3D11_VIEWPORT viewport;
    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
    D3D11_BLEND_DESC blendStateDescription;


    // Store the vsync setting.
    m_vsync_enabled = vsync;

    // Create a DirectX graphics interface factory.
    Utils::ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(m_factory.GetAddressOf())));

    // Retrieve information from the adapter.
    GetAdapterInformation();

    // Create the device and swap chain
    CreateDeviceAndSwapChain(screenWidth, screenHeight, hwnd);

    // Get the pointer to the back buffer.
    Utils::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferPtr)));

    // Create the render target view with the back buffer pointer.
    m_backBufferRT = std::make_shared<RenderTarget>(backBufferPtr, 0, m_device.Get());

    // Release pointer to the back buffer as we no longer need it.
    backBufferPtr->Release();
    backBufferPtr = nullptr;

    m_depthBuffer = std::make_shared<DepthBuffer>(screenWidth, screenHeight, 0, m_device.Get());

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;
    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the depth stencil state.
    Utils::ThrowIfFailed(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState));

    // Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
    // that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
    depthDisabledStencilDesc.DepthEnable = false;

    // Create the state using the device.
    Utils::ThrowIfFailed(m_device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState));

    // Set the depth stencil state.
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

    // Setup the raster description which will determine how and what polygons will be drawn.
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    Utils::ThrowIfFailed(m_device->CreateRasterizerState(&rasterDesc, &m_rasterState));

    // Now set the rasterizer state.
    m_deviceContext->RSSetState(m_rasterState.Get());
    
    // Setup the viewport for rendering.
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // Create the viewport.
    m_deviceContext->RSSetViewports(1, &viewport);

    

    // Create an alpha enabled blend state description.
    blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
    blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

    // Create the blend state using the description.
    Utils::ThrowIfFailed(m_device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState));

    // Modify the description to create an alpha disabled blend state description.
    blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

    // Create the second blend state using the description.
    Utils::ThrowIfFailed(m_device->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState));
}

void D3DClass::GetAdapterInformation()
{
    Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
    DXGI_ADAPTER_DESC adapterDesc;

    // Use the factory to create an adapter for the primary graphics interface (video card).
    Utils::ThrowIfFailed(m_factory->EnumAdapters(0, m_adapter.GetAddressOf()));

    // Get the adapter (video card) description.
    Utils::ThrowIfFailed(m_adapter->GetDesc(&adapterDesc));

    // Store the dedicated video card memory in megabytes.
    m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // Convert the name of the video card from wide to standard character length.
    std::wstring wideAdapterDescription(adapterDesc.Description);
    m_videoCardDescription = std::string(wideAdapterDescription.begin(), wideAdapterDescription.end());
}

void D3DClass::CreateDeviceAndSwapChain(int screenWidth, int screenHeight, HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;

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
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

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
    D3D11CreateDevice(m_adapter.Get(), D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevelsSupported, ARRAYSIZE(featureLevelsSupported), D3D11_SDK_VERSION,
        tempDevice.GetAddressOf(), NULL, m_deviceContext.GetAddressOf());
    // Now convert the device to a ID3D11Device1
    Utils::ThrowIfFailed(tempDevice.As(&m_device));
    // Remove the temp device
    tempDevice = nullptr;
    // Query the debug device from the device
    Utils::ThrowIfFailed(m_device.As(&m_debugDevice));

    // Create the swap chain
    m_factory->CreateSwapChainForHwnd(m_device.Get(), hwnd, &swapChainDesc, NULL, NULL, m_swapChain.GetAddressOf());
}


void D3DClass::Shutdown()
{
    // Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
    if(m_swapChain)
    {
        m_swapChain->SetFullscreenState(false, NULL);
    }

    m_alphaEnableBlendingState = nullptr;
    m_alphaDisableBlendingState = nullptr;
    m_depthDisabledStencilState = nullptr;
    m_rasterState = nullptr;
    m_depthStencilState = nullptr;
    m_depthStencilBuffer = nullptr;
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


void D3DClass::TurnZBufferOn()
{
    m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
    return;
}


void D3DClass::TurnZBufferOff()
{
    m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState.Get(), 1);
    return;
}


void D3DClass::TurnOnAlphaBlending()
{
    float blendFactor[4];
    

    // Setup the blend factor.
    blendFactor[0] = 0.0f;
    blendFactor[1] = 0.0f;
    blendFactor[2] = 0.0f;
    blendFactor[3] = 0.0f;
    
    // Turn on the alpha blending.
    m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState.Get(), blendFactor, 0xffffffff);

    return;
}


void D3DClass::TurnOffAlphaBlending()
{
    float blendFactor[4];
    

    // Setup the blend factor.
    blendFactor[0] = 0.0f;
    blendFactor[1] = 0.0f;
    blendFactor[2] = 0.0f;
    blendFactor[3] = 0.0f;
    
    // Turn off the alpha blending.
    m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState.Get(), blendFactor, 0xffffffff);

    return;
}