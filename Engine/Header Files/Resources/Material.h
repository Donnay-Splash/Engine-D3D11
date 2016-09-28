#pragma once
#include "ShaderPipeline.h"
#include "PipelineState.h"
#include "assimp\material.h"

class Material
{
public:
    using Ptr = std::shared_ptr<Material>;
    Material(ID3D11Device* device, ShaderPipeline::Ptr shaderPipeline);// , BlendState blendState = BlendState());

    void Load(aiMaterial* material);
    void Render(ID3D11DeviceContext* deviceContext);

private:
    ShaderPipeline::Ptr m_shaderPipeline;
    PipelineState::Ptr m_pipelineState;
};