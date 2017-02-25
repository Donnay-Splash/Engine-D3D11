#include "pch.h"
#include <Resources\Texture.h>
#include <DDSTextureLoader.h>
#include <Resources\ConstantBuffer.h>

// TODO: Tidy
namespace Engine
{

    Texture::Ptr Texture::CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t flags, DXGI_FORMAT format, ID3D11Device* device)
    {
        return CreateTextureArray(data, width, height, 1, flags, format, device);
    }

    Texture::Ptr Texture::CreateTextureArray(void* data, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t flags, DXGI_FORMAT format, ID3D11Device* device)
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

        D3D11_TEXTURE2D_DESC desc;
        SecureZeroMemory(&desc, sizeof(desc));
        desc.Height = height;
        desc.Width = width;
        desc.MipLevels = mipLevels;
        desc.ArraySize = arraySize;
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.MiscFlags = 0;
        desc.BindFlags = 0;

        if (bindToShader)
        {
            desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (bindToRenderTarget)
        {
            desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        }
        if (bindDepthStencil)
        {
            desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
        }

        // Required to use new since constructor is not accessible to std::make_shared
        return std::shared_ptr<Texture>(new Texture(data, desc, device));
    }

    Texture::Texture(void* data, D3D11_TEXTURE2D_DESC desc, ID3D11Device* device) :
        m_height(desc.Height), m_width(desc.Width), m_format(desc.Format), m_arraySize(desc.ArraySize)
    {
        D3D11_SUBRESOURCE_DATA* initData = nullptr;
        // Initialization data passed in
        if (data != nullptr)
        {
            uint32_t bytesPerPixel = static_cast<uint32_t>(Utils::DirectXHelpers::BitsPerPixel(m_format)) / 8;
            D3D11_SUBRESOURCE_DATA subresourceData;
            subresourceData.pSysMem = data;
            subresourceData.SysMemPitch = m_width * bytesPerPixel;
            subresourceData.SysMemSlicePitch = m_width * m_height * bytesPerPixel;
            initData = &subresourceData;
        }

        Utils::DirectXHelpers::ThrowIfFailed(device->CreateTexture2D(&desc, initData, m_texture.GetAddressOf()));

        if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0)
        {
            if (desc.Format == DXGI_FORMAT_R24G8_TYPELESS)
            {
                // Need to use separate format when binding depth for reading in shader
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                if (desc.ArraySize > 1)
                {
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                    srvDesc.Texture2DArray.ArraySize = desc.ArraySize;
                    srvDesc.Texture2DArray.FirstArraySlice = 0;
                    srvDesc.Texture2DArray.MipLevels = desc.MipLevels;
                    srvDesc.Texture2DArray.MostDetailedMip = 0;
                }
                else
                {
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MipLevels = desc.MipLevels;
                    srvDesc.Texture2D.MostDetailedMip = 0;
                }
                Utils::DirectXHelpers::ThrowIfFailed(device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srv.GetAddressOf()));
            }
            else
            {
                Utils::DirectXHelpers::ThrowIfFailed(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf()));
            }
        }
    }

    Texture::Ptr Texture::CreateTextureFromResource(ID3D11Texture2D* texture, uint32_t flags, ID3D11Device* device)
    {
        return std::shared_ptr<Texture>(new Texture(texture, flags, device));
    }

    // Create texture from D3D11 resource.
    Texture::Texture(ID3D11Texture2D* texture, uint32_t flags, ID3D11Device* device)
    {
        m_texture = texture;
        D3D11_TEXTURE2D_DESC textureDesc;
        m_texture->GetDesc(&textureDesc);

        m_height = textureDesc.Height;
        m_width = textureDesc.Width;
        m_format = textureDesc.Format;
        m_arraySize = textureDesc.ArraySize;
        bool bindToShader = (textureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0;

        if (bindToShader)
        {
            Utils::DirectXHelpers::ThrowIfFailed(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf()));
        }
    }

    Texture::Ptr Texture::CreateTextureFromFile(std::wstring filename, ID3D11Device* device)
    {
        return std::shared_ptr<Texture>(new Texture(device, filename.c_str()));
    }

    // Create a texture from a file
    Texture::Texture(ID3D11Device* device, const wchar_t* filename)
    {
        // Add support for additional file types.
        ID3D11Resource* subresource;
        Utils::DirectXHelpers::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, filename, &subresource, m_srv.GetAddressOf()));
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        m_srv->GetDesc(&desc);
        m_format = desc.Format;

        switch (desc.ViewDimension)
        {
        case D3D11_SRV_DIMENSION_TEXTURE2D:
        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
            Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
            D3D11_TEXTURE2D_DESC desc;
            tex2D->GetDesc(&desc);
            m_height = desc.Height;
            m_width = desc.Width;
            m_arraySize = desc.ArraySize;
            break;
        }
        default:
            // Unexpected view dimension received
            EngineAssert(false);
            break;
        }
    }

    Texture::Ptr Texture::CreateImportedTexture(const Utils::Loader::TextureData& importedTextureData, ID3D11Device* device)
    {
        return std::shared_ptr<Texture>(new Texture(device, importedTextureData));
    }

    // Create a texture from imported texture data
    Texture::Texture(ID3D11Device* device, const Utils::Loader::TextureData& importedTextureData)
    {
        ID3D11Resource* subresource;
        Utils::DirectXHelpers::ThrowIfFailed(DirectX::CreateDDSTextureFromMemory(device, importedTextureData.data.data(), importedTextureData.dataSize, &subresource, m_srv.GetAddressOf()));
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        m_srv->GetDesc(&desc);
        m_format = desc.Format;

        switch (desc.ViewDimension)
        {
        case D3D11_SRV_DIMENSION_TEXTURE2D:
        {
            Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
            Utils::DirectXHelpers::ThrowIfFailed(subresource->QueryInterface(tex2D.GetAddressOf()));
            D3D11_TEXTURE2D_DESC desc;
            tex2D->GetDesc(&desc);
            m_height = desc.Height;
            m_width = desc.Width;
            m_arraySize = desc.ArraySize;
            break;
        }
        default:
            // Unexpected view dimension received
            EngineAssert(false);
            break;
        }
    }

    Texture::Ptr Texture::CreateIdenticalTexture(Texture::Ptr const texture, ID3D11Device* device)
    {
        return CreateTextureArray(nullptr, texture->m_width, texture->m_height, texture->m_arraySize, TextureCreationFlags::BindShaderResource, texture->m_format, device);
    }

    void Texture::UploadData(ID3D11DeviceContext* deviceContext, uint32_t pipelineStage, uint32_t textureRegister)
    {
        if (pipelineStage & PipelineStage::Pixel)
        {
            deviceContext->PSSetShaderResources(textureRegister, 1, m_srv.GetAddressOf());
        }

        if (pipelineStage & PipelineStage::Vertex)
        {
            deviceContext->VSSetShaderResources(textureRegister, 1, m_srv.GetAddressOf());
        }
    }
}