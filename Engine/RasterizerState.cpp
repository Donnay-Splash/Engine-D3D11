#include "pch.h"
#include "RasterizerState.h"

RasterizerState::RasterizerState(ID3D11Device* device, D3D11_CULL_MODE cullMode = D3D11_CULL_BACK)
{
    // Setup the raster description which will determine how and what polygons will be drawn.
    m_rasterizerDesc.AntialiasedLineEnable = false;
    m_rasterizerDesc.CullMode = cullMode;
    m_rasterizerDesc.DepthBias = 0;
    m_rasterizerDesc.DepthBiasClamp = 0.0f;
    m_rasterizerDesc.DepthClipEnable = true;
    m_rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    m_rasterizerDesc.FrontCounterClockwise = false;
    m_rasterizerDesc.MultisampleEnable = false;
    m_rasterizerDesc.ScissorEnable = false;
    m_rasterizerDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    Utils::ThrowIfFailed(device->CreateRasterizerState(&m_rasterizerDesc, m_rasterizerState.GetAddressOf()));
}