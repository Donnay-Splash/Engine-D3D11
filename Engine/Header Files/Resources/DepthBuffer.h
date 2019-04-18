#pragma once

#include "Texture.h"

namespace Engine
{
    class DepthBuffer : public ShaderResource
    {
    public:
        using Ptr = std::shared_ptr<DepthBuffer>;
        DepthBuffer(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags);

        Texture::Ptr GetTexture() const { return m_texture; }
    private:
        Texture::Ptr m_texture;
    };
}