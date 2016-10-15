#include "pch.h"
#include <Resources\DepthState.h>

namespace Engine
{
    DepthState::DepthState(ID3D11Device* device, bool depthEnabled /*= true*/)
    {
        // Set up the description of the stencil state.
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        SecureZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
        depthStencilDesc.DepthEnable = depthEnabled;
        depthStencilDesc.DepthWriteMask = depthEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
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

        Utils::DirectXHelpers::ThrowIfFailed(device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf()));
    }

    void DepthState::UploadData(ID3D11DeviceContext * deviceContext)
    {
        deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
    }
}