#include "pch.h"
#include <Resources\RenderTarget.h>

namespace Engine
{
    RenderTarget::RenderTarget(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags, DXGI_FORMAT format, ID3D11Device* device)
    {
        creationFlags |= TextureCreationFlags::BindRenderTarget;
        m_texture = Texture::CreateTextureArray(nullptr, width, height, arraySize, creationFlags, format, device);
        Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), NULL, m_renderTargetView.GetAddressOf()));
    }

    RenderTarget::RenderTarget(ID3D11Texture2D* texture, uint32_t creationFlags, ID3D11Device* device)
    {
        m_texture = Texture::CreateTextureFromResource(texture, creationFlags, device);
        bool useSRGB = (creationFlags & TextureCreationFlags::SRGB) != 0;
        auto format = useSRGB ? Utils::DirectXHelpers::MakeSRGB(m_texture->GetFormat()) : m_texture->GetFormat();
        CD3D11_RENDER_TARGET_VIEW_DESC desc(D3D11_RTV_DIMENSION_TEXTURE2D, format);
        Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), &desc, m_renderTargetView.GetAddressOf()));
    }

    RenderTarget::~RenderTarget()
    {

    }
}