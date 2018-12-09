#pragma once

// Manages the state of the entire pipeline similar to a PSO in D3D12
#include "BlendState.h"
#include "DepthState.h"
#include "RasterizerState.h"

namespace Engine
{
    class PipelineState
    {
    public:
        using Ptr = std::shared_ptr<PipelineState>;
        PipelineState(BlendMode blendMode, D3D11_CULL_MODE cullMode, bool depthEnabled);

        void UploadData(ID3D11DeviceContext* deviceContext);

        void SetBlendState(BlendState::Ptr blendState) { m_blendState = blendState; }
        void SetDepthState(DepthState::Ptr depthState) { m_depthState = depthState; }
        void SetRasterizerState(RasterizerState::Ptr rasterizerState) { m_rasterizerState = rasterizerState; }

        BlendState::Ptr GetBlendState() const { return m_blendState; }
        DepthState::Ptr GetDepthState() const { return m_depthState; }
        RasterizerState::Ptr GetRasterizerState() const { return m_rasterizerState; }

    private:
        BlendState::Ptr m_blendState;
        DepthState::Ptr m_depthState;
        RasterizerState::Ptr m_rasterizerState;
    };
}