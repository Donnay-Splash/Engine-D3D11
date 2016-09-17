#include "pch.h"
#include "DepthState.h"

DepthState::DepthState(ID3D11Device* device, bool depthEnabled /*= true*/)
{
    InitializeDescription(depthEnabled);
    Utils::ThrowIfFailed(device->CreateDepthStencilState(&m_depthStencilDesc, m_depthStencilState.GetAddressOf()));
}

void DepthState::InitializeDescription(bool depthEnabled)
{
    // Set up the description of the stencil state.
    m_depthStencilDesc.DepthEnable = depthEnabled;
    m_depthStencilDesc.DepthWriteMask = depthEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    m_depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    m_depthStencilDesc.StencilEnable = true;
    m_depthStencilDesc.StencilReadMask = 0xFF;
    m_depthStencilDesc.StencilWriteMask = 0xFF;
    // Stencil operations if pixel is front-facing.
    m_depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    m_depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    m_depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    m_depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Stencil operations if pixel is back-facing.
    m_depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    m_depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    m_depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    m_depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
}