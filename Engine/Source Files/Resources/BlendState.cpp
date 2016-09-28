#include "pch.h"
#include <Resources\BlendState.h>

BlendState::BlendState(ID3D11Device* device, BlendMode initMode /* = BlendMode::Opaque*/)
{
    // Create an alpha enabled blend state description.
    D3D11_BLEND_DESC blendStateDescription;
    SecureZeroMemory(&blendStateDescription, sizeof(blendStateDescription));
    // Set the blend state for all bound rendertargets
    for (uint32_t i = 0; i < kRenderTargetMax; i++)
    {
        switch (initMode)
        {
        case BlendMode::Opaque:
            blendStateDescription.RenderTarget[i].BlendEnable = FALSE;
            blendStateDescription.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
            blendStateDescription.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
            blendStateDescription.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
            break;
        case BlendMode::Transparent:
            blendStateDescription.RenderTarget[i].BlendEnable = TRUE;
            blendStateDescription.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendStateDescription.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blendStateDescription.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
            break;
        default:
            // Invalid blendmode was given
            EngineAssert(false);
            break;
        }

        blendStateDescription.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateDescription.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendStateDescription.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDescription.RenderTarget[i].RenderTargetWriteMask = 0x0f;
    }

    // Create the second blend state using the description.
    Utils::DirectXHelpers::ThrowIfFailed(device->CreateBlendState(&blendStateDescription, m_blendState.GetAddressOf()));
}

void BlendState::SetBlendMode(BlendMode mode)
{
    // Not yet implemented.
    EngineAssert(false);
}

void BlendState::UploadData(ID3D11DeviceContext* deviceContext)
{
    deviceContext->OMSetBlendState(m_blendState.Get(), NULL, 0xffffffff);
}
