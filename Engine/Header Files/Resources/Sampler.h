#pragma once
#include "Sampler.h"

class Sampler
{
public:
    using Ptr = std::shared_ptr<Sampler>;
    Sampler(ID3D11Device* device,
            D3D11_FILTER filterMode,
            D3D11_TEXTURE_ADDRESS_MODE addressMode,
            float mipLODBias = 0.0f,
            UINT maxAnisotropy = 1,
            D3D11_COMPARISON_FUNC comparisonFunction = D3D11_COMPARISON_ALWAYS,
            Utils::Maths::Color borderColor = Utils::Maths::Color(DirectX::Colors::Black),
            float minLOD = 0.0f,
            float maxLOD = D3D11_FLOAT32_MAX);

    void UploadData(ID3D11DeviceContext* deviceContext, UINT inputSlot);

private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
};
