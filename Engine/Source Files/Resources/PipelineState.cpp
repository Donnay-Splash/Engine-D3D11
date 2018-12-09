#include "pch.h"
#include <Resources\PipelineState.h>

namespace Engine
{
    PipelineState::PipelineState(BlendMode blendMode, D3D11_CULL_MODE cullMode, bool depthEnabled)
    {
        m_blendState = std::make_shared<BlendState>(blendMode);
        m_rasterizerState = std::make_shared<RasterizerState>(cullMode);
        m_depthState = std::make_shared<DepthState>(depthEnabled);
    }

    void PipelineState::UploadData(ID3D11DeviceContext* deviceContext)
    {
        m_blendState->UploadData(deviceContext);
        m_rasterizerState->UploadData(deviceContext);
        m_depthState->UploadData(deviceContext);
    }
}
