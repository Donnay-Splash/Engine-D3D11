#pragma once

#include "pch.h"
#include <Resources\ShaderPipeline.h>

// TODO: Come up with a better sounding name
enum class ShaderName
{
	Uber = 0
};

class ShaderManager
{
public:
	using Ptr = std::shared_ptr<ShaderManager>;

	// TODO: eventually make shader manager a friend of the Renderer class
	// so that it can set this up without having to pass too much shit around.
	ShaderManager(ID3D11Device* device);

	ShaderPipeline::Ptr GetShaderPipeline(ShaderName);
private:
	using ShaderMap = std::map<ShaderName, ShaderPipeline::Ptr>;
	using ShaderMapObject = ShaderMap::value_type;
	void LoadCoreShaders(ID3D11Device* device);

private:
	ShaderMap m_shaderMap;
};

