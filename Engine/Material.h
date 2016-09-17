#pragma once
#include "ShaderPipeline.h"
#include "PipelineState.h"

class Material
{
public:
    using Ptr = std::shared_ptr<Material>;
    Material(ShaderPipeline::Ptr shaderPipeline);// , BlendState blendState = BlendState());

    void Render(ID3D11DeviceContext* deviceContext);

private:
    ShaderPipeline::Ptr m_shaderPipeline;
    PipelineState::Ptr m_pipelineState;
};