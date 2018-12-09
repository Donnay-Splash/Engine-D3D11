#include "pch.h"
#include <Resources\ShaderPipeline.h>

namespace Engine
{
    ShaderPipeline::ShaderPipeline(InputLayout::Ptr inputLayout,
        Shader::Ptr vertexShader,
        Shader::Ptr pixelShader /*= nullptr*/,
        Shader::Ptr geometryShader/* = nullptr*/) :
        m_inputLayout(inputLayout),
        m_vertexShader(vertexShader),
        m_geometryShader(geometryShader),
        m_pixelShader(pixelShader)
    {
        // The vertex shader cannot be null
        EngineAssert(m_vertexShader != nullptr);
        EngineAssert(m_vertexShader->GetType() == Shader::Type::Vertex);

        // Validate that the input layout for the vertex shader is correct
        m_vertexShader->ValidateInputSignature(inputLayout, device);

        if (m_pixelShader != nullptr)
        {
            EngineAssert(m_pixelShader->GetType() == Shader::Type::Pixel);
        }
        else
        {
            // Create empty shader
            m_pixelShader = Shader::CreateNullShader(Shader::Type::Pixel);
        }

        if (m_geometryShader != nullptr)
        {
            EngineAssert(m_geometryShader->GetType() == Shader::Type::Geometry);
        }
        else
        {
            // Create empty shader
            m_geometryShader = Shader::CreateNullShader(Shader::Type::Geometry);
        }
    }

    void ShaderPipeline::UploadData(ID3D11DeviceContext* deviceContext)
    {
        m_inputLayout->UploadData(deviceContext);
        m_vertexShader->UploadData(deviceContext);
        m_geometryShader->UploadData(deviceContext);
        m_pixelShader->UploadData(deviceContext);
    }
}