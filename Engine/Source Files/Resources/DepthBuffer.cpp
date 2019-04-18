#include "pch.h"
#include <Resources\DepthBuffer.h>
#include "d3dclass.h"

namespace Engine
{
    DepthBuffer::DepthBuffer(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags)
    {
        creationFlags |= TextureCreationFlags::BindDepthStencil;
        m_texture = Texture::CreateTextureArray(nullptr, width, height, arraySize, creationFlags, DXGI_FORMAT_D24_UNORM_S8_UINT);

        D3D12_DEPTH_STENCIL_VIEW_DESC desc;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.Flags = D3D12_DSV_FLAG_NONE;
        if (arraySize > 1)
        {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.ArraySize = arraySize;
            desc.Texture2DArray.FirstArraySlice = 0;
            desc.Texture2DArray.MipSlice = 0;
        }
        else
        {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
        }

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		m_descriptor = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		device->CreateDepthStencilView(m_texture->GetResource(), &desc, m_descriptor->CPUHandle);
    }
}