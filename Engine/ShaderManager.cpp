#include "pch.h"
#include "ShaderManager.h"
#include "ShaderPipeline.h"
#include "InputLayout.h"

namespace VMShader
{
	#include "Shaders\Compiled shaders\manipulation.ps.h"
	#include "Shaders\Compiled shaders\manipulation.vs.h"
}

ShaderManager::ShaderManager(ID3D11Device* device)
{
	LoadCoreShaders(device);
}

void ShaderManager::LoadCoreShaders(ID3D11Device* device)
{
	// Here we create the D3D11 shader objects to be placed into the bundle.
	// At this time we will also create the input layouts for each shader bundle.

	// Load the VM shaders
	{

		InputLayout::Ptr layout = std::make_shared<InputLayout>(InputElement::Position | InputElement::Normal0 | InputElement::TexCoord0);
		Shader::Ptr vertexShader = std::make_shared<Shader>(Shader::Type::Vertex, VMShader::g_VSMain, sizeof(VMShader::g_VSMain), device);
		Shader::Ptr pixelShader = std::make_shared<Shader>(Shader::Type::Pixel, VMShader::g_PSMain, sizeof(VMShader::g_PSMain), device);
		ShaderPipeline::Ptr shaderPipeline = std::make_shared<ShaderPipeline>(vertexShader, pixelShader, layout, device);

		m_shaderMap.insert(ShaderMapObject(ShaderName::VertexManipulation, shaderPipeline));
	}
}

ShaderPipeline::Ptr ShaderManager::GetShaderPipeline(ShaderName shaderName)
{
	auto shaderPipelineIt = m_shaderMap.find(shaderName);

	// The shader should exist within the map if it has an enum value.
	// If passed an incorrect enum value then we need to assert.
	EngineAssert(shaderPipelineIt != m_shaderMap.end());

	return shaderPipelineIt->second;
}