#pragma once
#include <Resources\ShaderPipeline.h>
#include <Resources\PipelineState.h>
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    // TODO: Add public properties so we can manipulate effect variables via UI
    template<typename T>
    class PostEffect
    {
    public:
        using Ptr = std::shared_ptr<PostEffect<T>>;
        PostEffect(ShaderPipeline::Ptr shaderPipeline) :
            m_shaderPipeline(shaderPipeline)
        {
            m_pipelineState = std::make_shared<PipelineState>(BlendMode::Opaque, D3D11_CULL_NONE, false);
            m_effectConstants = std::make_shared<ConstantBuffer<T>>(PipelineStage::Pixel);
        }

        void Render(ID3D11DeviceContext* deviceContext)
        {
            m_shaderPipeline->UploadData(deviceContext);
            m_pipelineState->UploadData(deviceContext);
            m_effectConstants->SetData(m_effectData);
            m_effectConstants->UploadData(deviceContext);
        }

        void SetEffectData(const T& effectData) { m_effectData = effectData; }
        T GetEffectData() const { return m_effectData; }

    private:
        ShaderPipeline::Ptr m_shaderPipeline;
        PipelineState::Ptr m_pipelineState;
        std::shared_ptr<ConstantBuffer<T>> m_effectConstants;
        T m_effectData;
    };
}