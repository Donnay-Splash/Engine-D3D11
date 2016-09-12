#pragma once

#include "pch.h"
#include "Texture.h"
class RenderTarget
{
public:
	using Ptr = std::shared_ptr<RenderTarget>;
	
	RenderTarget();
	~RenderTarget();

	void Initialize();
	Texture::Ptr GetTexture() { return m_texture; }

private:
	Texture::Ptr m_texture;

};