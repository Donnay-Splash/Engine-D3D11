#pragma once

#include "pch.h"

// TODO: Come up with a better sounding name
enum class ShaderName
{
	VertexManipulation = 0
};

struct ShaderBundle
{
	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
};

class ShaderManager
{
public:
	using Ptr = std::shared_ptr<ShaderManager>;

	// TODO: eventually make shader manager a friend of the Renderer class
	// so that it can set this up without having to pass too much shit around.
	ShaderManager(ID3D11Device* device);

	ShaderBundle GetShaderBundle(ShaderName);
private:
	void LoadCoreShaders(ID3D11Device* device);

private:
	std::map<ShaderName, ShaderBundle> m_shaderMap;
};

