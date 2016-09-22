#include "pch.h"
#include "RasterizerState.h"

RasterizerState::RasterizerState(ID3D11Device* device, D3D11_CULL_MODE cullMode /*= D3D11_CULL_BACK*/)
{
    // Setup the raster description which will determine how and what polygons will be drawn.
    D3D11_RASTERIZER_DESC rasterizerDesc;
    SecureZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.CullMode = cullMode;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    Utils::DirectXHelpers::ThrowIfFailed(device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf()));
}

void RasterizerState::UploadData(ID3D11DeviceContext* deviceContext)
{
    deviceContext->RSSetState(m_rasterizerState.Get());
}
