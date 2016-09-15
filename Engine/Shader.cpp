#include "pch.h"
#include "Shader.h"

Shader::Shader(Shader::Type type, const BYTE* shaderCode, const SIZE_T shaderLength, ID3D11Device* device)
	: m_type(type), m_shaderCode(shaderCode), m_shaderLength(shaderLength)
{
	switch (m_type)
	{
		case Type::Vertex:
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			device->CreateVertexShader(shaderCode, shaderLength, nullptr, vertexShader.GetAddressOf());
			vertexShader.As(&m_shader);
			break;
		}
		case Type::Pixel:
		{
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			device->CreatePixelShader(shaderCode, shaderLength, nullptr, pixelShader.GetAddressOf());
			pixelShader.As(&m_shader);
			break;
		}
	}
}

void Shader::ValidateInputSignature(InputLayout::Ptr inputLayout, ID3D11Device* device)
{
	// Cannot create an input layout from anything but a vertex shader.
	EngineAssert(m_type == Type::Vertex);
	UINT numElements = static_cast<UINT>(inputLayout->m_inputElements.size());
	device->CreateInputLayout(inputLayout->m_inputElements.data(), numElements, m_shaderCode, m_shaderLength, inputLayout->m_inputLayout.GetAddressOf());
}

void Shader::UploadData(ID3D11DeviceContext* deviceContext)
{
	switch (m_type)
	{
		case Type::Vertex:
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			m_shader.As(&vertexShader);
			deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
			break;
		}
		case Type::Pixel:
		{
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			m_shader.As(&pixelShader);
			deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
			break;
		}
	}
}