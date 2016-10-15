#include "pch.h"
#include <Resources\PipelineState.h>

namespace Engine
{
    PipelineState::PipelineState(ID3D11Device* device, BlendMode blendMode, D3D11_CULL_MODE cullMode, bool depthEnabled)
    {
        m_blendState = std::make_shared<BlendState>(device, blendMode);
        m_rasterizerState = std::make_shared<RasterizerState>(device, cullMode);
        m_depthState = std::make_shared<DepthState>(device, depthEnabled);
    }

    void PipelineState::UploadData(ID3D11DeviceContext* deviceContext)
    {
        m_blendState->UploadData(deviceContext);
        m_rasterizerState->UploadData(deviceContext);
        m_depthState->UploadData(deviceContext);
    }
}
