#include "pch.h"
#include <Resources\RenderTarget.h>
#include "d3dclass.h"

namespace Engine
{
    RenderTarget::RenderTarget(uint32_t width, uint32_t height, uint32_t arraySize, uint32_t creationFlags, DXGI_FORMAT format)
    {
        creationFlags |= TextureCreationFlags::BindRenderTarget;
        m_texture = Texture::CreateTextureArray(nullptr, width, height, arraySize, creationFlags, format);
		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		m_descriptor = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(m_texture->GetResource(), NULL, m_descriptor->CPUHandle);
    }

    RenderTarget::RenderTarget(Texture::Ptr texture, uint32_t creationFlags, uint32_t mipSlice, uint32_t arraySlice)
    {
        EngineAssert(texture != nullptr);
        m_texture = texture;
        bool useSRGB = (creationFlags & TextureCreationFlags::SRGB) != 0;
        auto format = useSRGB ? Utils::DirectXHelpers::MakeSRGB(m_texture->GetFormat()) : m_texture->GetFormat();
        D3D12_RESOURCE_DESC textureDesc = m_texture->GetResource()->GetDesc();

        EngineAssert(arraySlice < textureDesc.DepthOrArraySize);
        bool isTextureArray = textureDesc.DepthOrArraySize > 1;
        D3D12_RTV_DIMENSION rtvDimension = isTextureArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;

        auto maxDimension = std::max(m_texture->GetHeight(), m_texture->GetWidth());
        auto maxMip = textureDesc.MipLevels > 0 ? textureDesc.MipLevels : 1 + static_cast<uint32_t>(log2(maxDimension));
        EngineAssert(mipSlice <= maxMip);

		D3D12_RENDER_TARGET_VIEW_DESC desc = {};
		desc.ViewDimension = rtvDimension;
		desc.Format = format;
        
        if (isTextureArray)
        {
            desc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize;
            desc.Texture2DArray.FirstArraySlice = arraySlice;
            desc.Texture2DArray.MipSlice = mipSlice;
        }
        else
        {
            desc.Texture2D.MipSlice = mipSlice;
        }

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		m_descriptor = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(m_texture->GetResource(), &desc, m_descriptor->CPUHandle);
    }

    RenderTarget::RenderTarget(ID3D12Resource* texture, uint32_t creationFlags)
    {
        m_texture = Texture::CreateTextureFromResource(texture, creationFlags);

		// TODO: Re-implement SRGB for DX12
        /*bool useSRGB = (creationFlags & TextureCreationFlags::SRGB) != 0;
        auto format = useSRGB ? Utils::DirectXHelpers::MakeSRGB(m_texture->GetFormat()) : m_texture->GetFormat();
        D3D12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = format;*/

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		m_descriptor = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		device->CreateRenderTargetView(m_texture->GetResource(), nullptr, m_descriptor->CPUHandle);
    }

    RenderTarget::~RenderTarget()
    {

    }
}