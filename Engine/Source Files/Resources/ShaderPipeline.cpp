#include "pch.h"
#include <Resources\ShaderPipeline.h>

namespace Engine
{
    ShaderPipeline::ShaderPipeline(Shader::Ptr vertexShader,
        Shader::Ptr pixelShader,
        Shader::Ptr geometryShader,
        InputLayout::Ptr inputLayout,
        ID3D11Device* device) :
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

        // Create the pixel shader from the buffer.
        if (m_pixelShader != nullptr)
        {
            EngineAssert(m_pixelShader->GetType() == Shader::Type::Pixel);
        }

        if (m_geometryShader != nullptr)
        {
            EngineAssert(m_geometryShader->GetType() == Shader::Type::Geometry);
        }
    }

    void ShaderPipeline::UploadData(ID3D11DeviceContext* deviceContext)
    {
        m_inputLayout->UploadData(deviceContext);
        m_vertexShader->UploadData(deviceContext);

        // TODO: Find a better way to bind shaders to the pipeline.
        // Should be able to call UploadData but the shader sets a nullptr
        // through the device context.
        if (m_pixelShader != nullptr)
        {
            m_pixelShader->UploadData(deviceContext);
        }
        else
        {
            deviceContext->PSSetShader(nullptr, nullptr, 0);
        }

        if (m_geometryShader != nullptr)
        {
            m_geometryShader->UploadData(deviceContext);
        }
        else
        {
            deviceContext->GSSetShader(nullptr, nullptr, 0);
        }
    }
}