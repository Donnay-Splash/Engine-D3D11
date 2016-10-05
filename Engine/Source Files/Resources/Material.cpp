#include "pch.h"
#include <Resources\Material.h>
#include <memory>

Material::Material(ID3D11Device* device, ShaderPipeline::Ptr shaderPipeline) :
    m_shaderPipeline (shaderPipeline)
{
    m_pipelineState = std::make_shared<PipelineState>(device, BlendMode::Opaque, D3D11_CULL_NONE, true);
    m_materialConstants = std::make_shared<ConstantBuffer<MaterialConstants>>(PipelineStage::Pixel, device);

    // Create a basic sampler for all of the textures
    // TODO: create a sampler when a texture is added.
    for (int i = 0; i < static_cast<uint32_t>(TextureType::Count); i++)
    {
        auto sampler = std::make_shared<Sampler>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
        m_samplers[i] = sampler;
    }
}


Texture::Ptr Material::GetTexture(TextureType type) const
{
    // Count is an invalid value
    if (type == TextureType::Count) return nullptr;
    size_t index = static_cast<size_t>(type);
    return m_textures[index];
}

void Material::SetTexture(TextureType type, Texture::Ptr texture)
{
    // Count is an invalid value.
    if (type == TextureType::Count) return;
    size_t index = static_cast<size_t>(type);
    m_textures[index] = texture;
    // Create a sampler for the texture
}

void Material::SetDiffuseColor(const Utils::Maths::Color& color)
{
    m_diffuseAndOpacity.x = color.x;
    m_diffuseAndOpacity.y = color.y;
    m_diffuseAndOpacity.z = color.z;
}

void Material::SetSpecularColor(const Utils::Maths::Color& color)
{
    m_specularAndSmoothness.x = color.x;
    m_specularAndSmoothness.y = color.y;
    m_specularAndSmoothness.z = color.z;
}

void Material::Render(ID3D11DeviceContext* deviceContext)
{
    m_shaderPipeline->UploadData(deviceContext);
    m_pipelineState->UploadData(deviceContext);
    m_materialConstants->SetData({ m_diffuseAndOpacity, m_specularAndSmoothness, m_emissiveColor });
    m_materialConstants->UploadData(deviceContext);

    for (uint32_t i = 0; i < static_cast<uint32_t>(TextureType::Count); i++)
    {
        auto texture = m_textures[i];
        if (texture)
        {
            texture->UploadData(deviceContext, PipelineStage::Pixel, i);
            auto sampler = m_samplers[i];
            EngineAssert(sampler != nullptr);
            sampler->UploadData(deviceContext, i);
        }
    }
}