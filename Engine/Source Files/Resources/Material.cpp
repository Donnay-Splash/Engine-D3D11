#include "pch.h"
#include <Resources\Material.h>
#include <memory>

namespace Engine
{
    Material::Material(ID3D11Device* device, ShaderPipeline::Ptr shaderPipeline) : SceneElement(L"Material"),
        m_shaderPipeline(shaderPipeline)
    {
        m_pipelineState = std::make_shared<PipelineState>(device, BlendMode::Opaque, D3D11_CULL_NONE, true);
        m_solidFillState = std::make_shared<RasterizerState>(device, D3D11_CULL_NONE, D3D11_FILL_SOLID);
        m_wireFrameState = std::make_shared<RasterizerState>(device, D3D11_CULL_NONE, D3D11_FILL_WIREFRAME);

        m_materialConstants = std::make_shared<ConstantBuffer<MaterialConstants>>(PipelineStage::Pixel, device);

        // Create a basic sampler for all of the textures
        // TODO: create a sampler when a texture is added.
        for (int i = 0; i < static_cast<uint32_t>(TextureType::Count); i++)
        {
            auto sampler = std::make_shared<Sampler>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
            m_samplers[i] = sampler;
        }

        RegisterPublicProperties();
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

        switch (type)
        {
        case TextureType::Diffuse:
            m_materialData.hasDiffuseTexture = static_cast<float>(texture != nullptr);
            break;
        case TextureType::Specular:
            m_materialData.hasSpecularTexture = static_cast<float>(texture != nullptr);
            break;
        case TextureType::Emissive:
            m_materialData.hasEmissiveTexture = static_cast<float>(texture != nullptr);
            break;
        case TextureType::Normal:
            m_materialData.hasNormalTexture = static_cast<float>(texture != nullptr);
            break;
        case TextureType::Smoothness:
            m_materialData.hasSmoothnessTexture = static_cast<float>(texture != nullptr);
            break;
        case TextureType::Opacity:
            m_materialData.hasOpacityTexture = static_cast<float>(texture != nullptr);
            break;
        case TextureType::BakedAO:
            m_materialData.hasAOTexture = static_cast<float>(texture != nullptr);
            break;
        default:
            break;
        }
    }

    void Material::SetDiffuseColor(const Utils::Maths::Color& color)
    {
        m_materialData.diffuseColorAndOpacity.x = color.x;
        m_materialData.diffuseColorAndOpacity.y = color.y;
        m_materialData.diffuseColorAndOpacity.z = color.z;
    }

    void Material::SetSpecularColor(const Utils::Maths::Color& color)
    {
        m_materialData.specularColorAndSmoothness.x = color.x;
        m_materialData.specularColorAndSmoothness.y = color.y;
        m_materialData.specularColorAndSmoothness.z = color.z;
    }

    void Material::Render(ID3D11DeviceContext* deviceContext)
    {
        m_shaderPipeline->UploadData(deviceContext);
        m_pipelineState->UploadData(deviceContext);
        m_materialConstants->SetData(m_materialData);
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

    void Material::SetWireframeEnabled(bool enabled)
    {
        if (enabled)
        {
            m_pipelineState->SetRasterizerState(m_wireFrameState);
        }
        else
        {
            m_pipelineState->SetRasterizerState(m_solidFillState);
        }
    }

    void Material::RegisterPublicProperties()
    {
        auto smoothnessGetter = [this]() { return GetSmoothness(); };
        auto smoothnessSetter = [this](float smoothness) { SetSmoothness(smoothness); };
        RegisterScalarProperty(L"Smoothness", smoothnessGetter, smoothnessSetter, 0.0f, 1.0f);

        auto opacityGetter = [this]() { return GetOpacity(); };
        auto opacitySetter = [this](float opacity) { SetOpacity(opacity); };
        RegisterScalarProperty(L"Opacity", opacityGetter, opacitySetter, 0.0f, 1.0f);

        // TODO Add colour setter and getter
    }
}