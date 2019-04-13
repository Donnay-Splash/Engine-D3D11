#include "pch.h"
#include <Resources\Material.h>
#include <memory>

namespace Engine
{
    Material::Material(ShaderPipeline::Ptr shaderPipeline) : SceneElement(L"Material"),
        m_shaderPipeline(shaderPipeline)
    {
        m_pipelineState = std::make_shared<PipelineState>(BlendMode::Opaque, D3D11_CULL_BACK, true);
        m_materialConstants = std::make_shared<ConstantBuffer<MaterialConstants>>(PipelineStage::Pixel);

        // Only need one sampler for all textures. No need to complicate it.
        m_sampler = std::make_shared<Sampler>(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

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
		// TODO DX12: pass through command list
        //m_shaderPipeline->UploadData(deviceContext);
        m_pipelineState->UploadData(deviceContext);
        m_materialConstants->SetData(m_materialData);
        m_materialConstants->UploadData(deviceContext);
        m_sampler->UploadData(deviceContext, 0);
        for (uint32_t i = 0; i < static_cast<uint32_t>(TextureType::Count); i++)
        {
            auto texture = m_textures[i];
            if (texture)
            {
				IMPLEMENT_FOR_DX12(texture->UploadData(deviceContext, PipelineStage::Pixel, i);)
            }
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

        auto emissiveGetter = [this]() { return m_materialData.emissiveIntensity; };
        auto emissiveSetter = [this](float value) { m_materialData.emissiveIntensity = value; };
        RegisterScalarProperty(L"Emissive Intensity", emissiveGetter, emissiveSetter, 0.0f, 10.0f);

        auto diffuseEnabledGetter = [this]() { return m_materialData.hasDiffuseTexture == 1.0f ? true : false; };
        auto diffuseEnabledSetter = [this](bool enabled) { m_materialData.hasDiffuseTexture = enabled ? 1.0f : 0.0f; };
        RegisterBooleanProperty(L"Diffuse Enabled", diffuseEnabledGetter, diffuseEnabledSetter);

        // TODO: Fix this
        auto smoothnessEnabledGetter = [this]() { return m_materialData.hasSpecularTexture == 1.0f ? true : false; };
        auto smoothnessEnabledSetter = [this](bool enabled) { m_materialData.hasSpecularTexture = enabled ? 1.0f : 0.0f; };
        RegisterBooleanProperty(L"Smoothness Enabled", smoothnessEnabledGetter, smoothnessEnabledSetter);

        auto normalEnabledGetter = [this]() { return m_materialData.hasNormalTexture == 1.0f ? true : false; };
        auto normalEnabledSetter = [this](bool enabled) { m_materialData.hasNormalTexture = enabled ? 1.0f : 0.0f; };
        RegisterBooleanProperty(L"Normal Mapping Enabled", normalEnabledGetter, normalEnabledSetter);

        // TODO Add colour setter and getter
    }
}