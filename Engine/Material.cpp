#include "pch.h"
#include "Material.h"

Material::Material(ShaderPipeline::Ptr shaderPipeline) :
    m_shaderPipeline (shaderPipeline)
{
    
}

void Material::Render(ID3D11DeviceContext* deviceContext)
{
    m_shaderPipeline->UploadData(deviceContext);
}