#pragma once
#include "Texture.h"

namespace Engine
{
    class RenderTarget : public ShaderResource
    {
    public:
        using Ptr = std::shared_ptr<RenderTarget>;

        RenderTarget(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags, DXGI_FORMAT format);
        // Create a render target to draw to the texture at the given mip and array slice
        RenderTarget(Texture::Ptr texture, uint32_t creationFlags, uint32_t mipSlice, uint32_t arraySlice);
        RenderTarget(ID3D12Resource* texture, uint32_t creationFlags);
        ~RenderTarget();

        Texture::Ptr GetTexture() const { return m_texture; }

        uint32_t GetHeight() const { return m_texture->GetHeight(); }
        uint32_t GetWidth() const { return m_texture->GetWidth(); }

    private:
        Texture::Ptr m_texture;
    };
}