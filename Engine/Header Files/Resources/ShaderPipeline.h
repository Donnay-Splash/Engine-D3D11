#pragma once
#include "InputLayout.h"
#include "Shader.h"

namespace Engine
{
	// TODO DX12: Add colour
	namespace InputElement
	{
		static const uint32_t Position = 0x0001;
		static const uint32_t TexCoord0 = 0x0002;
		static const uint32_t Normal0 = 0x0004;
		static const uint32_t Tangents = 0x0008;
		static const uint32_t Colour = 0x0010;
	};

    class ShaderPipeline
    {
    public:
        using Ptr = std::shared_ptr<ShaderPipeline>;
        ShaderPipeline(uint32_t inputFlags,
            Shader vertexShader,
            Shader pixelShader = Shader(),
            Shader geometryShader = Shader(),
			bool transparent = false);

        void UploadData(ID3D12GraphicsCommandList* commandList);

	private:
		std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout(uint32_t inputFlags) const;

    private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    };
}