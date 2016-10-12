#include "pch.h"
#include <Resources\Texture.h>
#include <DDSTextureLoader.h>
#include <Resources\ConstantBuffer.h>

Texture::Texture(void* data, uint32_t width, uint32_t height, uint32_t flags, DXGI_FORMAT format, ID3D11Device* device) :
    m_height(height), m_width(width), m_format(format)
{
    bool bindToShader = (flags & TextureCreationFlags::BindShaderResource) != 0;
    bool bindToRenderTarget = (flags & TextureCreationFlags::BindShaderResource) != 0;
    bool bindDepthStencil = (flags & TextureCreationFlags::BindDepthStencil) != 0;

    D3D11_TEXTURE2D_DESC desc;
    SecureZeroMemory(&desc, sizeof(desc));
    desc.Height = height;
    desc.Width = width;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = m_format;
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

    D3D11_SUBRESOURCE_DATA* initData = nullptr;
    // Initialization data passed in
    if (data != nullptr)
    {
        // Presumes texture data coming in is 32 bits per pixel.
        uint32_t bytesPerPixel = static_cast<uint32_t>(Utils::DirectXHelpers::BitsPerPixel(format)) / 8;
        D3D11_SUBRESOURCE_DATA subresourceData;
        subresourceData.pSysMem = data;
        subresourceData.SysMemPitch = width * bytesPerPixel;
        subresourceData.SysMemSlicePitch = width * height * bytesPerPixel;
        initData = &subresourceData;
    }

    Utils::DirectXHelpers::ThrowIfFailed(device->CreateTexture2D(&desc, initData, m_texture.GetAddressOf()));

    if (bindToShader)
    {
        Utils::DirectXHelpers::ThrowIfFailed(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf()));
    }
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
    bool bindToShader = (textureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0;

    if (bindToShader)
    {
        Utils::DirectXHelpers::ThrowIfFailed(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf()));
    }
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
        break;
    }
    default:
        // Unexpected view dimension received
        EngineAssert(false);
        break;
    }
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
        break;
    }
    default:
        // Unexpected view dimension received
        EngineAssert(false);
        break;
    }
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