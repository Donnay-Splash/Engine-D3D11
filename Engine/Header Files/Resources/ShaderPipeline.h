#pragma once
#include "InputLayout.h"
#include "Shader.h"

namespace Engine
{
    class ShaderPipeline
    {
    public:
        using Ptr = std::shared_ptr<ShaderPipeline>;
        ShaderPipeline(ID3D11Device* device,
            InputLayout::Ptr inputLayout,
            Shader::Ptr vertexShader,
            Shader::Ptr pixelShader = nullptr,
            Shader::Ptr geometryShader = nullptr);

        void UploadData(ID3D11DeviceContext* deviceContext);

    private:
        Shader::Ptr m_vertexShader;
        Shader::Ptr m_geometryShader;
        Shader::Ptr m_pixelShader;
        InputLayout::Ptr m_inputLayout;
    };
}