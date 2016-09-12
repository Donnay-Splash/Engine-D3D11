#include "pch.h"
#include <DDSTextureLoader.h>
#include "Texture.h"

Texture::Texture()
{
	m_texture = nullptr;
}


Texture::Texture(const Texture& other)
{
}


Texture::~Texture()
{
}

void Texture::Create(void* data, uint32_t height, uint32_t width, uint32_t flags, ID3D11Device* device /*.format*/)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Height = height;
	desc.Width = width;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA* initData = nullptr;
	// Initialization data passed in
	if (data != nullptr)
	{
		uint32_t bytesPerPixel = 4;
		D3D11_SUBRESOURCE_DATA subresourceData;
		subresourceData.pSysMem = data;
		subresourceData.SysMemPitch = width * bytesPerPixel;
		subresourceData.SysMemSlicePitch = width * height * bytesPerPixel;
		initData = &subresourceData;
	}

	Utils::ThrowIfFailed(device->CreateTexture2D(&desc, initData, m_texture.GetAddressOf()));
	Utils::ThrowIfFailed(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_srv.GetAddressOf()));
}

void Texture::CreateFromFile(ID3D11Device* device, const wchar_t* filename)
{
	// Add support for additional file types.
    ID3D11Resource* subresource;
    Utils::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, filename, &subresource, m_srv.GetAddressOf()));
}


