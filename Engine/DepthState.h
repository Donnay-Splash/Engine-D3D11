#pragma once
#include "pch.h"

class DepthState
{
public:
    using Ptr = std::shared_ptr<DepthState>;
    DepthState(ID3D11Device* device, bool depthEnabled);

    void UploadData(ID3D11DeviceContext* deviceContext);

private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
};