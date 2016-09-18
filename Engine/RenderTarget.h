#pragma once

#include "pch.h"
#include "Texture.h"
class RenderTarget
{
public:
    using Ptr = std::shared_ptr<RenderTarget>;
    
    RenderTarget(uint32_t width, uint32_t height, uint32_t creationFlags, DXGI_FORMAT format, ID3D11Device* device);
    RenderTarget(ID3D11Texture2D* texture, uint32_t creationFlags, ID3D11Device* device);
    ~RenderTarget();

    Texture::Ptr GetTexture() const { return m_texture; }
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRTV() const { return m_renderTargetView; }

    uint32_t GetHeight() const { return m_texture->GetHeight(); }
    uint32_t GetWidth() const { return m_texture->GetWidth(); }

private:
    Texture::Ptr m_texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
};