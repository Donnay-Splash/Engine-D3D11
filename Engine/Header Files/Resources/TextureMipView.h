#pragma once
#include "Texture.h"

namespace Engine
{
    // Handles creation of shader resources to view specific mip levels of a
    // texture. This is handy for custom downsampling. Allowing reading
    // and writing to different mips of a texture at the same time.
    class TextureMipView
    {
    public:
        using Ptr = std::shared_ptr<TextureMipView>;
        TextureMipView(ID3D11Device* device, Texture::Ptr texture, uint32_t mipLevels = 0);
        
        void UploadData(ID3D11DeviceContext* deviceContext, uint32_t pipelineStage, uint32_t textureRegister);
        void SetCurrentMip(uint32_t currentMip) { EngineAssert(currentMip < m_mipLevels); m_currentMip = currentMip; }
        Texture::Ptr GetTexture() const { return m_texture; };
        uint32_t GetMipCount() const { return m_mipLevels; }

    private:
        Texture::Ptr m_texture;

        std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_mipSRVs;
        uint32_t m_mipLevels = 0;
        uint32_t m_currentMip = 0;
    };
}