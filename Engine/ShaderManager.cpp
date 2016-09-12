#include "pch.h"
#include "ShaderManager.h"

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
		// Create the vertex shader from the buffer.
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vmVertexShader;
		Utils::ThrowIfFailed(device->CreateVertexShader(VMShader::g_VSMain, sizeof(VMShader::g_VSMain), NULL, vmVertexShader.GetAddressOf()));

		// Create the pixel shader from the buffer.
		Microsoft::WRL::ComPtr<ID3D11PixelShader> vmPixelShader;
		Utils::ThrowIfFailed(device->CreatePixelShader(VMShader::g_PSMain, sizeof(VMShader::g_PSMain), NULL, vmPixelShader.GetAddressOf()));

		// Load the input layout.
		// Create the vertex input layout description.
		// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
		//TODO: Create input layout class to simplify this.
		D3D11_INPUT_ELEMENT_DESC polygonLayout[3];

		polygonLayout[0].SemanticName = "POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "TEXCOORD";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		polygonLayout[2].SemanticName = "NORMAL";
		polygonLayout[2].SemanticIndex = 0;
		polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[2].InputSlot = 0;
		polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[2].InstanceDataStepRate = 0;

		// Get a count of the elements in the layout.
		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		// Create the vertex input layout.
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Utils::ThrowIfFailed(device->CreateInputLayout(polygonLayout, numElements, VMShader::g_VSMain, sizeof(VMShader::g_VSMain), inputLayout.GetAddressOf()));

		// Create the ShaderBundle
		ShaderBundle vmBundle{vmVertexShader, vmPixelShader, inputLayout};

		m_shaderMap[ShaderName::VertexManipulation] = vmBundle;
	}
}

ShaderBundle ShaderManager::GetShaderBundle(ShaderName shaderName)
{
	auto shaderBundleIt = m_shaderMap.find(shaderName);

	// The shader should exist within the map if it has an enum value.
	// If passed an incorrect enum value then we need to assert.
	EngineAssert(shaderBundleIt != m_shaderMap.end());

	return shaderBundleIt->second;
}