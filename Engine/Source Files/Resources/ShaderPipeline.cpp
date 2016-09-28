#include "pch.h"
#include <Resources\ShaderPipeline.h>

ShaderPipeline::ShaderPipeline(Shader::Ptr vertexShader,
	Shader::Ptr pixelShader,
	InputLayout::Ptr inputLayout,
	ID3D11Device* device) :
	m_inputLayout(inputLayout),
	m_vertexShader(vertexShader),
	m_pixelShader(pixelShader)
{
	// The vertex shader cannot be null
	EngineAssert(m_vertexShader != nullptr);
	EngineAssert(m_vertexShader->GetType() == Shader::Type::Vertex);

	// Validate that the input layout for the vertex shader is correct
	m_vertexShader->ValidateInputSignature(inputLayout, device);

	// Create the pixel shader from the buffer.
	if (m_pixelShader != nullptr)
	{
		EngineAssert(m_pixelShader->GetType() == Shader::Type::Pixel);
	}

}

void ShaderPipeline::UploadData(ID3D11DeviceContext* deviceContext)
{
	m_inputLayout->UploadData(deviceContext);
	m_vertexShader->UploadData(deviceContext);
	m_pixelShader->UploadData(deviceContext);
}