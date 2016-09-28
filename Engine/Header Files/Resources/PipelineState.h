#pragma once

// Manages the state of the entire pipeline similar to a PSO in D3D12
#include "BlendState.h"
#include "DepthState.h"
#include "RasterizerState.h"

class PipelineState
{
public:
    using Ptr = std::shared_ptr<PipelineState>;
    PipelineState(ID3D11Device* device, BlendMode blendMode, D3D11_CULL_MODE cullMode, bool depthEnabled);

    void UploadData(ID3D11DeviceContext* deviceContext);

private:
    BlendState::Ptr m_blendState;
    DepthState::Ptr m_depthState;
    RasterizerState::Ptr m_rasterizerState;
};