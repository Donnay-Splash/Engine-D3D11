#pragma once
#include "InputLayout.h"

namespace Engine
{
    // forward declarations
    class ShaderPipeline;

    class Shader
    {
    public:
        using Ptr = std::shared_ptr<Shader>;
        enum class Type
        {
            Vertex,
            Pixel,
            Geometry/*,
            Domain,
            Hull,
            Compute*/
        };
        Shader(Type type, const BYTE* shaderCode, const SIZE_T shaderLength);

        Type GetType() const { return m_type; }

    private:
        void ValidateInputSignature(InputLayout::Ptr layout);
        void UploadData(ID3D11DeviceContext* deviceContext);
        static Shader::Ptr CreateNullShader(Type type);
        Shader(Type type);

    private:
        const Type m_type;
        const BYTE* m_shaderCode;
        const SIZE_T m_shaderLength;
        Microsoft::WRL::ComPtr<ID3D11DeviceChild> m_shader;
        friend class ShaderPipeline;
    };
}