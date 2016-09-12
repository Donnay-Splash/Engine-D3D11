#pragma once
#include "pch.h"

class Texture
{
public:
	using Ptr = std::shared_ptr<Texture>;
	Texture();
	Texture(const Texture&);
	~Texture();

	// Creates a texture from a set of parameters.
	void Create(void* data, uint32_t height, uint32_t width, uint32_t flags, ID3D11Device* device /*.format*/);

	// Creates a texture from an image file.
	void CreateFromFile(ID3D11Device* device, const wchar_t* filename);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture() { return m_texture; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() { return m_srv; }

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

	float height;
	float width;
	// format

};
