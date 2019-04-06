#pragma once
#include "InputLayout.h"

namespace Engine
{
    // forward declarations
    class ShaderPipeline;

	enum class ShaderType
	{
		Null,
		Vertex,
		Pixel,
		Geometry/*,
		Domain,
		Hull,
		Compute*/
	};

    struct Shader
    {
        Shader() :
			Type(ShaderType::Null), ShaderCode(nullptr), ShaderLength(0) {}

		Shader(ShaderType type, const BYTE* shaderCode, const SIZE_T shaderLength) :
			Type(type), ShaderCode(shaderCode), ShaderLength(shaderLength) {}

        const ShaderType Type;
        const BYTE* ShaderCode;
        const SIZE_T ShaderLength;
    };
}