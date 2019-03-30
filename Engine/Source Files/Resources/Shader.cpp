#include "pch.h"
#include <Resources\Shader.h>

namespace Engine
{
    Shader::Shader(Shader::Type type, const BYTE* shaderCode, const SIZE_T shaderLength)
        : m_type(type), m_shaderCode(shaderCode), m_shaderLength(shaderLength)
    {
        switch (m_type)
        {
        case Type::Vertex:
        {
            Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			IMPLEMENT_FOR_DX12(device->CreateVertexShader(shaderCode, shaderLength, nullptr, vertexShader.GetAddressOf());)
            vertexShader.As(&m_shader);
            break;
        }
        case Type::Pixel:
        {
            Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			IMPLEMENT_FOR_DX12(device->CreatePixelShader(shaderCode, shaderLength, nullptr, pixelShader.GetAddressOf());)
            pixelShader.As(&m_shader);
            break;
        }
        case Type::Geometry:
        {
            Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
			IMPLEMENT_FOR_DX12(device->CreateGeometryShader(shaderCode, shaderLength, nullptr, geometryShader.GetAddressOf());)
            geometryShader.As(&m_shader);
            break;
        }
        }
    }

    Shader::Shader(Shader::Type type) : 
        m_type(type), m_shaderLength(0), m_shaderCode(nullptr)
    {

    }

    void Shader::ValidateInputSignature(InputLayout::Ptr inputLayout)
    {
        // Cannot create an input layout from anything but a vertex shader.
        EngineAssert(m_type == Type::Vertex);
        UINT numElements = static_cast<UINT>(inputLayout->m_inputElements.size());
		IMPLEMENT_FOR_DX12(device->CreateInputLayout(inputLayout->m_inputElements.data(), numElements, m_shaderCode, m_shaderLength, inputLayout->m_inputLayout.GetAddressOf());)
    }

    void Shader::UploadData(ID3D11DeviceContext* deviceContext)
    {
        switch (m_type)
        {
        case Type::Vertex:
        {
            Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
            if (m_shader != nullptr)
            {
                m_shader.As(&vertexShader);
            }
            deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
            break;
        }
        case Type::Pixel:
        {
            Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
            if (m_shader != nullptr)
            {
                m_shader.As(&pixelShader);
            }
            deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
            break;
        }
        case Type::Geometry:
        {
            Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader;
            if (m_shader != nullptr)
            {
                m_shader.As(&geometryShader);
            }
            deviceContext->GSSetShader(geometryShader.Get(), nullptr, 0);
            break;
        }
        }
    }

    Shader::Ptr Shader::CreateNullShader(Shader::Type type)
    {
        // Cannot create null vertex shader.
        EngineAssert(type != Shader::Type::Vertex);

        Shader::Ptr newShader = std::shared_ptr<Shader>(new Shader(type));
        newShader->m_shader = nullptr;

        return newShader;
    }
}