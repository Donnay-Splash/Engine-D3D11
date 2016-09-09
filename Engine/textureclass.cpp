#include "pch.h"
#include <DDSTextureLoader.h>
#include "textureclass.h"

TextureClass::TextureClass()
{
	m_texture = 0;
}


TextureClass::TextureClass(const TextureClass& other)
{
}


TextureClass::~TextureClass()
{
}


bool TextureClass::Initialize(ID3D11Device* device, WCHAR* filename)
{
    ID3D11Resource* subresource;
    Utils::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device, filename, &subresource, &m_texture));

	return true;
}


void TextureClass::Shutdown()
{
	// Release the texture resource.
	if(m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}