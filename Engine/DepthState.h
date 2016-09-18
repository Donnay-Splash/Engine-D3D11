#pragma once
#include "pch.h"

class DepthState
{
public:
    using Ptr = std::shared_ptr<DepthState>;
    DepthState(ID3D11Device* device, bool depthEnabled);

    void UploadData(ID3D11DeviceContext* deviceContext);

private:
    void InitializeDescription(bool depthEnabled);

private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    D3D11_DEPTH_STENCIL_DESC m_depthStencilDesc;
};