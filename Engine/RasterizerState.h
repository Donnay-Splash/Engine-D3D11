#pragma once

#include "pch.h"

class RasterizerState
{
public:
    using Ptr = std::shared_ptr<RasterizerState>;
    RasterizerState(ID3D11Device* device, D3D11_CULL_MODE cullMode = D3D11_CULL_BACK);

private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    D3D11_RASTERIZER_DESC m_rasterizerDesc;
};