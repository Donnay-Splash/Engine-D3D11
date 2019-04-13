#include "pch.h"
#include <Resources\Texture.h>
#include <DDSTextureLoader.h>
#include <Resources\ConstantBuffer.h>
#include <Header Files\d3dclass.h>

// TODO: Tidy
namespace Engine
{

    Texture::Ptr Texture::CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t flags, DXGI_FORMAT format)
    {
        return CreateTextureArray(data, width, height, 1, flags, format);
    }

    Texture::Ptr Texture::CreateTextureArray(void* data, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t flags, DXGI_FORMAT format)
    {
        EngineAssert(width > 0);
        EngineAssert(height > 0);
        EngineAssert(arraySize > 0 && arraySize < D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);

        bool bindToShader = (flags & TextureCreationFlags::BindShaderResource) != 0;
        bool bindToRenderTarget = (flags & TextureCreationFlags::BindRenderTarget) != 0;
        bool bindDepthStencil = (flags & TextureCreationFlags::BindDepthStencil) != 0;
        bool generateMIPs = (flags & TextureCreationFlags::GenerateMIPs) != 0;
        uint32_t mipLevels = 1;
        if (generateMIPs)
        {
            mipLevels = 1 + static_cast<uint32_t>(log2(std::max(width, height)));
        }

        if ((bindDepthStencil && bindToShader) || (format == DXGI_FORMAT_D24_UNORM_S8_UINT && bindToShader))
        {
            // If we are using it for depth and reading from a shader we need to be specific about format specification
            format = DXGI_FORMAT_R24G8_TYPELESS;
        }

        D3D12_RESOURCE_DESC desc;
        SecureZeroMemory(&desc, sizeof(desc));
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Height = height;
        desc.Width = width;
        desc.MipLevels = mipLevels;
        desc.DepthOrArraySize = arraySize;
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        if (!bindToShader)
        {
			// Do we want this?
            desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
        if (bindToRenderTarget)
        {
            desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if (bindDepthStencil)
        {
            desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }

        // Required to use new since constructor is not accessible to std::make_shared
        return std::shared_ptr<Texture>(new Texture(data, desc));
    }

    Texture::Texture(void* data, D3D12_RESOURCE_DESC desc) : GPUResource(D3D12_RESOURCE_STATE_COMMON),
        m_height(desc.Height), m_width(desc.Width), m_format(desc.Format), m_arraySize(desc.DepthOrArraySize), m_mipLevels(desc.MipLevels)
    {

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf())));

        if (data != nullptr)
        {
            uint32_t bytesPerPixel = static_cast<uint32_t>(Utils::DirectXHelpers::BitsPerPixel(m_format)) / 8;
			D3D12_SUBRESOURCE_DATA subresourceData;
			subresourceData.pData = data;
			subresourceData.RowPitch = m_width * bytesPerPixel;
			subresourceData.SlicePitch = subresourceData.RowPitch * m_height;

			D3DClass::Instance()->UploadSubresourceData(GetRequiredIntermediateSize(m_resource.Get(), 0, 1), &subresourceData, m_resource.Get(), m_usageState);
        }

		// If not denying shader acces then create SRV
        if ((desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0)
        {
			m_descriptor = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            if (desc.Format == DXGI_FORMAT_R24G8_TYPELESS)
            {
                // Need to use separate format when binding depth for reading in shader
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                if (desc.DepthOrArraySize > 1)
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    srvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
                    srvDesc.Texture2DArray.FirstArraySlice = 0;
                    srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
                    srvDesc.Texture2DArray.MostDetailedMip = 0;
                }
                else
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = desc.MipLevels;
                    srvDesc.Texture2D.MostDetailedMip = 0;
                }
				device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_descriptor->CPUHandle);
            }
            else
            {
				device->CreateShaderResourceView(m_resource.Get(), nullptr, m_descriptor->CPUHandle);
            }
        }
    }

    Texture::Ptr Texture::CreateTextureFromResource(ID3D12Resource* texture, uint32_t flags)
    {
        return std::shared_ptr<Texture>(new Texture(texture, flags));
    }

    // Create texture from D3D11 resource.
    Texture::Texture(ID3D12Resource* texture, uint32_t flags) : GPUResource(D3D12_RESOURCE_STATE_COMMON)
    {
        m_resource = texture;
		D3D12_RESOURCE_DESC textureDesc = m_resource->GetDesc();

        m_height = textureDesc.Height;
        m_width = textureDesc.Width;
        m_format = textureDesc.Format;
        m_arraySize = textureDesc.DepthOrArraySize;
        m_mipLevels = textureDesc.MipLevels;
        bool bindToShader = (textureDesc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0;

        if (bindToShader)
        {
			ID3D12Device* device = D3DClass::Instance()->GetDevice();
			m_descriptor = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			device->CreateShaderResourceView(m_resource.Get(), nullptr, m_descriptor->CPUHandle);
        }
    }

    Texture::Ptr Texture::CreateTextureFromFile(std::wstring filename)
    {
        return std::shared_ptr<Texture>(new Texture(filename.c_str()));
    }

    // Create a texture from a file
    Texture::Texture(const wchar_t* filename) : GPUResource(D3D12_RESOURCE_STATE_COMMON)
    {
		// Implement for Texture TK 12
		EngineAssert(false);


        // Add support for additional file types.
  //      ID3D11Resource* subresource = nullptr;
		//IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, filename, &subresource, m_srv.GetAddressOf()));)
  //      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  //      m_srv->GetDesc(&srvDesc);
  //      m_format = srvDesc.Format;

  //      switch (srvDesc.ViewDimension)
  //      {
  //      case D3D11_SRV_DIMENSION_TEXTURE2D:
  //      {
  //          Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
  //          Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
		//	D3D12_RESOURCE_DESC texDesc;
  //          tex2D->GetDesc(&texDesc);
  //          m_height = texDesc.Height;
  //          m_width = texDesc.Width;
  //          m_arraySize = texDesc.DepthOrArraySize;
  //          m_mipLevels = texDesc.MipLevels;
  //          break;
  //      }
  //      case D3D11_SRV_DIMENSION_TEXTURECUBE:
  //      {
  //          Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
  //          Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
		//	D3D12_RESOURCE_DESC texDesc;
  //          tex2D->GetDesc(&texDesc);
  //          m_height = texDesc.Height;
  //          m_width = texDesc.Width;
  //          m_arraySize = texDesc.DepthOrArraySize;
  //          m_mipLevels = texDesc.MipLevels;
  //          m_isCube = true;
  //          break;
  //      }
  //      default:
  //          // Unexpected view dimension received
  //          EngineAssert(false);
  //          break;
  //      }
    }

    Texture::Ptr Texture::CreateImportedTexture(const Utils::Loader::TextureData& importedTextureData)
    {
        return std::shared_ptr<Texture>(new Texture(importedTextureData.data.data(), importedTextureData.dataSize));
    }

    Texture::Ptr Texture::CreateTextureFromMemory(const uint8_t* data, uint64_t byteCount, bool generateMips /*= false*/)
    {
        if (generateMips)
        {
			// TODO: RE-implement for Dx12
			EngineAssert(false);
			//return std::shared_ptr<Texture>(new Texture(d3d->GetDeviceContext(), data, byteCount));
        }
        else
        {
			return std::shared_ptr<Texture>(new Texture(data, byteCount));
        }
		return nullptr;
    }

    // Create a texture from imported texture data
    Texture::Texture(const uint8_t* data, uint64_t byteCount) : GPUResource(D3D12_RESOURCE_STATE_COMMON)
    {
		// Implement for Texture TK 12
		EngineAssert(false);

  //      ID3D11Resource* subresource = nullptr;
		//IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(DirectX::CreateDDSTextureFromMemory(device, data, byteCount, &subresource, m_srv.GetAddressOf()));)
  //      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  //      m_srv->GetDesc(&srvDesc);
  //      m_format = srvDesc.Format;

  //      switch (srvDesc.ViewDimension)
  //      {
  //      case D3D11_SRV_DIMENSION_TEXTURE2D:
  //      {
  //          Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
  //          Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
		//	D3D12_RESOURCE_DESC texDesc;
  //          tex2D->GetDesc(&texDesc);
  //          m_height = texDesc.Height;
  //          m_width = texDesc.Width;
  //          m_arraySize = texDesc.DepthOrArraySize;
  //          m_mipLevels = texDesc.MipLevels;
  //          break;
  //      }
  //      case D3D11_SRV_DIMENSION_TEXTURECUBE:
  //      {
  //          Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
  //          Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
		//	D3D12_RESOURCE_DESC texDesc;
  //          tex2D->GetDesc(&texDesc);
  //          m_height = texDesc.Height;
  //          m_width = texDesc.Width;
  //          m_arraySize = texDesc.DepthOrArraySize;
  //          m_mipLevels = texDesc.MipLevels;
  //          m_isCube = true;
  //          break;
  //      }
  //      default:
  //          // Unexpected view dimension received
  //          EngineAssert(false);
  //          break;
  //      }
    }

    // Create a texture from imported texture data and auto generate mips
    Texture::Texture(ID3D12GraphicsCommandList* commandLsit, const uint8_t* data, uint64_t byteCount) : GPUResource(D3D12_RESOURCE_STATE_COMMON)
    {
		// Implement for Texture TK 12
		EngineAssert(false);

  //      ID3D11Resource* subresource = nullptr;
		//IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(DirectX::CreateDDSTextureFromMemory(device, context, data, byteCount, &subresource, m_srv.GetAddressOf()));)
  //      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
  //      m_srv->GetDesc(&srvDesc);
  //      m_format = srvDesc.Format;

  //      switch (srvDesc.ViewDimension)
  //      {
  //      case D3D11_SRV_DIMENSION_TEXTURE2D:
  //      {
  //          Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
  //          Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
		//	D3D12_RESOURCE_DESC texDesc;
  //          tex2D->GetDesc(&texDesc);
  //          m_height = texDesc.Height;
  //          m_width = texDesc.Width;
  //          m_arraySize = texDesc.DepthOrArraySize;
  //          m_mipLevels = texDesc.MipLevels;
  //          break;
  //      }
  //      case D3D11_SRV_DIMENSION_TEXTURECUBE:
  //      {
  //          Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
  //          Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
		//	D3D12_RESOURCE_DESC texDesc;
  //          tex2D->GetDesc(&texDesc);
  //          m_height = texDesc.Height;
  //          m_width = texDesc.Width;
  //          m_arraySize = texDesc.DepthOrArraySize;
  //          m_mipLevels = texDesc.MipLevels;
  //          m_isCube = true;
  //          break;
  //      }
  //      default:
  //          // Unexpected view dimension received
  //          EngineAssert(false);
  //          break;
  //      }
    }

    Texture::Ptr Texture::CreateIdenticalTexture(Texture::Ptr const texture)
    {
        return CreateTextureArray(nullptr, texture->m_width, texture->m_height, texture->m_arraySize, TextureCreationFlags::BindShaderResource, texture->m_format);
    }

    void Texture::UploadData(ID3D12GraphicsCommandList* commandList, uint32_t textureRegister)
    {
		commandList->SetGraphicsRootShaderResourceView(textureRegister, m_resource->GetGPUVirtualAddress());
    }
}