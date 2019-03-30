#include "pch.h"
#include <Resources\RenderTarget.h>

namespace Engine
{
    RenderTarget::RenderTarget(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags, DXGI_FORMAT format)
    {
        creationFlags |= TextureCreationFlags::BindRenderTarget;
        m_texture = Texture::CreateTextureArray(nullptr, width, height, arraySize, creationFlags, format);
		IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), NULL, m_renderTargetView.GetAddressOf()));)
        D3D11_RENDER_TARGET_VIEW_DESC desc;
        m_renderTargetView->GetDesc(&desc);
    }

    RenderTarget::RenderTarget(Texture::Ptr texture, uint32_t creationFlags, uint32_t mipSlice, uint32_t arraySlice)
    {
        EngineAssert(texture != nullptr);
        m_texture = texture;
        bool useSRGB = (creationFlags & TextureCreationFlags::SRGB) != 0;
        auto format = useSRGB ? Utils::DirectXHelpers::MakeSRGB(m_texture->GetFormat()) : m_texture->GetFormat();
        D3D11_TEXTURE2D_DESC textureDesc;
        m_texture->GetTexture()->GetDesc(&textureDesc);

        EngineAssert(arraySlice < textureDesc.ArraySize);
        bool isTextureArray = textureDesc.ArraySize > 1;
        auto rtvDimension = isTextureArray ? D3D11_RTV_DIMENSION_TEXTURE2DARRAY : D3D11_RTV_DIMENSION_TEXTURE2D;

        auto maxDimension = std::max(m_texture->GetHeight(), m_texture->GetWidth());
        auto maxMip = textureDesc.MipLevels > 0 ? textureDesc.MipLevels : 1 + static_cast<uint32_t>(log2(maxDimension));
        EngineAssert(mipSlice <= maxMip);

        CD3D11_RENDER_TARGET_VIEW_DESC desc(rtvDimension, format);
        
        if (isTextureArray)
        {
            desc.Texture2DArray.ArraySize = textureDesc.ArraySize;
            desc.Texture2DArray.FirstArraySlice = arraySlice;
            desc.Texture2DArray.MipSlice = mipSlice;
        }
        else
        {
            desc.Texture2D.MipSlice = mipSlice;
        }

		IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), &desc, m_renderTargetView.GetAddressOf()));)
    }

    RenderTarget::RenderTarget(ID3D11Texture2D* texture, uint32_t creationFlags)
    {
        m_texture = Texture::CreateTextureFromResource(texture, creationFlags);
        bool useSRGB = (creationFlags & TextureCreationFlags::SRGB) != 0;
        auto format = useSRGB ? Utils::DirectXHelpers::MakeSRGB(m_texture->GetFormat()) : m_texture->GetFormat();
        CD3D11_RENDER_TARGET_VIEW_DESC desc(D3D11_RTV_DIMENSION_TEXTURE2D, format);
		IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), &desc, m_renderTargetView.GetAddressOf()));)
    }

    RenderTarget::~RenderTarget()
    {

    }
}