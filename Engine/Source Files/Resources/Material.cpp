#include "pch.h"
#include <Resources\Material.h>

Material::Material(ID3D11Device* device, ShaderPipeline::Ptr shaderPipeline) :
    m_shaderPipeline (shaderPipeline)
{
    m_pipelineState = std::make_shared<PipelineState>(device, BlendMode::Opaque, D3D11_CULL_NONE, true);
}

//void Material::Load(aiMaterial* material)
//{
//
//}

void Material::Render(ID3D11DeviceContext* deviceContext)
{
    m_shaderPipeline->UploadData(deviceContext);
    m_pipelineState->UploadData(deviceContext);
}