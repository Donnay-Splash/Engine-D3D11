#include "pch.h"
#include "Sampler.h"

Sampler::Sampler(ID3D11Device* device,
    D3D11_FILTER filterMode,
    D3D11_TEXTURE_ADDRESS_MODE addressMode,
    float mipLODBias /*= 0.0f*/,
    UINT maxAnisotropy /*= 1*/,
    D3D11_COMPARISON_FUNC comparisonFunction /*= D3D11_COMPARISON_ALWAYS*/,
    Utils::Maths::Color borderColor /*= Utils::Maths::Color(DirectX::Colors::Black)*/,
    float minLOD /*= 0.0f*/,
    float maxLOD /*= D3D11_FLOAT32_MAX*/)
{
    D3D11_SAMPLER_DESC samplerDesc;
    // Sampler and blend states should be able to be created easier. Create some defaults at start up and then hand them out.
    // Create a texture sampler state description.
    samplerDesc.Filter = filterMode;
    samplerDesc.AddressU = addressMode;
    samplerDesc.AddressV = addressMode;
    samplerDesc.AddressW = addressMode;
    samplerDesc.MipLODBias = mipLODBias;
    samplerDesc.MaxAnisotropy = maxAnisotropy;
    samplerDesc.ComparisonFunc = comparisonFunction;
    samplerDesc.BorderColor[0] = borderColor.x;
    samplerDesc.BorderColor[1] = borderColor.y;
    samplerDesc.BorderColor[2] = borderColor.z;
    samplerDesc.BorderColor[3] = borderColor.w;
    samplerDesc.MinLOD = minLOD;
    samplerDesc.MaxLOD = maxLOD;

    // Create the texture sampler state.
    Utils::ThrowIfFailed(device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf()));
}

void Sampler::UploadData(ID3D11DeviceContext* deviceContext, UINT inputSlot)
{
    deviceContext->PSSetSamplers(inputSlot, 1, m_samplerState.GetAddressOf());
}