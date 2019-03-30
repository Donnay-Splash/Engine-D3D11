#include "pch.h"
#include <Resources\DepthBuffer.h>

namespace Engine
{
    DepthBuffer::DepthBuffer(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags)
    {
        creationFlags |= TextureCreationFlags::BindDepthStencil;
        m_texture = Texture::CreateTextureArray(nullptr, width, height, arraySize, creationFlags, DXGI_FORMAT_D24_UNORM_S8_UINT);

        D3D11_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.Flags = 0;
        if (arraySize > 1)
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.ArraySize = arraySize;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.MipSlice = 0;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }
		IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(device->CreateDepthStencilView(m_texture->GetTexture().Get(), &desc, m_dsv.GetAddressOf()));)
    }
}