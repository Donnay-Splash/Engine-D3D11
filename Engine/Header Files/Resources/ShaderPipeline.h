#pragma once
#include "pch.h"
#include "InputLayout.h"
#include "Shader.h"

class ShaderPipeline
{
public:
	using Ptr = std::shared_ptr<ShaderPipeline>;
	ShaderPipeline(Shader::Ptr vertexShader,
		Shader::Ptr pixelShader,
		InputLayout::Ptr inputLayout,
		ID3D11Device* device);

	void UploadData(ID3D11DeviceContext* deviceContext);
	
private:
	Shader::Ptr m_vertexShader;
	Shader::Ptr m_pixelShader;
	InputLayout::Ptr m_inputLayout;
};