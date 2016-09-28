#pragma once

namespace InputElement
{
	static const uint32_t Position = 0x0001;
	static const uint32_t TexCoord0 = 0x0002;
	static const uint32_t Normal0 = 0x0004;
};

// forward declaration
class Shader;
class ShaderPipeline;

class InputLayout
{
public:
	using Ptr = std::shared_ptr<InputLayout>;
	InputLayout(const uint32_t& inputFlags);

private:
	void UploadData(ID3D11DeviceContext* deviceContext);

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_inputElements;

	friend class Shader;
	friend class ShaderPipeline;
};