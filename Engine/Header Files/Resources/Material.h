#pragma once
#include "ConstantBuffer.h"
#include "ConstantBufferLayouts.h"
#include "PipelineState.h"
#include "Sampler.h"
#include "ShaderPipeline.h"
#include "Texture.h"
#include <array>
#include <Utils\Loader\Data.h>


namespace Engine
{
    class Material : public SceneElement
    {
    public:
        enum class TextureType
        {
            Diffuse = 0,
            Specular = 1,
            Emissive = 2,
            Normal = 3,
            Smoothness = 4,
            Opacity = 5,
            BakedAO = 6,
            Count
        };
    public:
        using Ptr = std::shared_ptr<Material>;
        Material(ID3D11Device* device, ShaderPipeline::Ptr shaderPipeline);// , BlendState blendState = BlendState());

        void Render(ID3D11DeviceContext* deviceContext);

        Texture::Ptr GetTexture(TextureType type) const;
        void SetTexture(TextureType type, Texture::Ptr texture);

        Utils::Maths::Color GetDiffuseColor() const { return m_materialData.diffuseColorAndOpacity; }
        Utils::Maths::Color GetSpecularColor() const { return m_materialData.specularColorAndSmoothness; }
        Utils::Maths::Color GetEmissiveColor() const { return m_materialData.emissiveColor; }
        float GetOpacity() const { return m_materialData.diffuseColorAndOpacity.w; }
        float GetSmoothness() const { return m_materialData.specularColorAndSmoothness.w; }

        void SetDiffuseColor(const Utils::Maths::Color& color);
        void SetSpecularColor(const Utils::Maths::Color& color);
        void SetEmissiveColor(const Utils::Maths::Color& color) { m_materialData.emissiveColor = color; };
        void SetOpacity(float opacity) { m_materialData.diffuseColorAndOpacity.w = opacity; }
        void SetSmoothness(float smoothness) { m_materialData.specularColorAndSmoothness.w = smoothness; }

        void SetWireframeEnabled(bool enabled);

    private:
        void RegisterPublicProperties();

    private:
        ShaderPipeline::Ptr m_shaderPipeline;
        RasterizerState::Ptr m_solidFillState;
        RasterizerState::Ptr m_wireFrameState;
        PipelineState::Ptr m_pipelineState;
        ConstantBuffer<MaterialConstants>::Ptr m_materialConstants;

        MaterialConstants m_materialData;
        std::array<Texture::Ptr, static_cast<size_t>(TextureType::Count)> m_textures;
        std::array<Sampler::Ptr, static_cast<size_t>(TextureType::Count)> m_samplers;
    };
}