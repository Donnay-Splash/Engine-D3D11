#pragma once
#include "pch.h"
#include "InputLayout.h"

// forward declarations
class ShaderPipeline;

class Shader
{
public:
	using Ptr = std::shared_ptr<Shader>;
	enum class Type
	{
		Vertex,
		Pixel/*,
		Geometry,
		Domain,
		Hull,
		Compute*/
	};
	Shader(Type type, const BYTE* shaderCode, const SIZE_T shaderLength, ID3D11Device* device);

	Type GetType() const { return m_type; }

private:
	void ValidateInputSignature(InputLayout::Ptr layout, ID3D11Device* device);
	void SetData(ID3D11DeviceContext* deviceContext);

private:
	const Type m_type;
	const BYTE* m_shaderCode;
	const SIZE_T m_shaderLength;
	Microsoft::WRL::ComPtr<ID3D11DeviceChild> m_shader;
	friend class ShaderPipeline;
};