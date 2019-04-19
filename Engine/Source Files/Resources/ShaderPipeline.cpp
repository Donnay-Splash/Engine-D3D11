#include "pch.h"
#include <Resources\ShaderPipeline.h>
#include "d3dclass.h"

namespace Engine
{
	ShaderPipeline::ShaderPipeline(uint32_t inputFlags,
		Shader vertexShader,
		Shader pixelShader /*= empty*/,
		Shader geometryShader/* = empty*/)
	{
		// Vailidate input
		EngineAssert(vertexShader.ShaderCode != nullptr || vertexShader.Type == ShaderType::Vertex);
		EngineAssert(pixelShader.ShaderCode == nullptr || pixelShader.Type == ShaderType::Pixel);
		EngineAssert(geometryShader.ShaderCode == nullptr || geometryShader.Type == ShaderType::Geometry);

		auto inputElements = GetInputLayout(inputFlags);
		auto device = D3DClass::Instance()->GetDevice();

		// Create empty root signature for now.
		{
			// TODO: Pass this in constructor
			// Create constant buffer root parameter
			CD3DX12_DESCRIPTOR_RANGE ranges[1];
			CD3DX12_ROOT_PARAMETER rootParameters[1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
			rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

			CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
			rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, flags);

			Microsoft::WRL::ComPtr<ID3DBlob> signature;
			Microsoft::WRL::ComPtr<ID3DBlob> error;
			Utils::DirectXHelpers::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
			Utils::DirectXHelpers::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		}

		// Create pipeline state;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElements.data(), (UINT)inputElements.size() };
		// Not valid need to create an empty root sig. Ideally we can get this from the shader?
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.ShaderCode, vertexShader.ShaderLength);
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.ShaderCode, pixelShader.ShaderLength);
		psoDesc.GS = CD3DX12_SHADER_BYTECODE(geometryShader.ShaderCode, geometryShader.ShaderLength);
		// We'll eventually need to make it easier to define and create the permutations of each PSO we want.
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = false;
		psoDesc.DepthStencilState.StencilEnable = false;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		Utils::DirectXHelpers::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> ShaderPipeline::GetInputLayout(uint32_t inputFlags) const
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
		UINT inputSlot = 0;
		if (inputFlags & InputElement::Position)
		{
			D3D12_INPUT_ELEMENT_DESC element;
			SecureZeroMemory(&element, sizeof(element));
			element.SemanticName = "POSITION";
			element.SemanticIndex = 0;
			element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			element.InputSlot = inputSlot;
			element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;

			inputElements.push_back(element);
			inputSlot++;
		}

		if (inputFlags & InputElement::Normal0)
		{
			D3D12_INPUT_ELEMENT_DESC element;
			SecureZeroMemory(&element, sizeof(element));
			element.SemanticName = "NORMAL";
			element.SemanticIndex = 0;
			element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			element.InputSlot = inputSlot;
			element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;

			inputElements.push_back(element);
			inputSlot++;
		}

		if (inputFlags & InputElement::TexCoord0)
		{
			D3D12_INPUT_ELEMENT_DESC element;
			SecureZeroMemory(&element, sizeof(element));
			element.SemanticName = "TEXCOORD";
			element.SemanticIndex = 0;
			element.Format = DXGI_FORMAT_R32G32_FLOAT;
			element.InputSlot = inputSlot;
			element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;

			inputElements.push_back(element);
			inputSlot++;
		}

		if (inputFlags & InputElement::Tangents)
		{
			D3D12_INPUT_ELEMENT_DESC tangentsDesc;
			SecureZeroMemory(&tangentsDesc, sizeof(tangentsDesc));
			tangentsDesc.SemanticName = "TANGENT";
			tangentsDesc.SemanticIndex = 0;
			tangentsDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			tangentsDesc.InputSlot = inputSlot;
			tangentsDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			tangentsDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			tangentsDesc.InstanceDataStepRate = 0;

			inputElements.push_back(tangentsDesc);
			inputSlot++;

			D3D12_INPUT_ELEMENT_DESC bitangentsDesc;
			SecureZeroMemory(&bitangentsDesc, sizeof(bitangentsDesc));
			bitangentsDesc.SemanticName = "BINORMAL";
			bitangentsDesc.SemanticIndex = 0;
			bitangentsDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			bitangentsDesc.InputSlot = inputSlot;
			bitangentsDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			bitangentsDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			bitangentsDesc.InstanceDataStepRate = 0;

			inputElements.push_back(bitangentsDesc);
			inputSlot++;
		}

		if (inputFlags & InputElement::Colour)
		{
			D3D12_INPUT_ELEMENT_DESC element;
			SecureZeroMemory(&element, sizeof(element));
			element.SemanticName = "COLOR";
			element.SemanticIndex = 0;
			element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			element.InputSlot = inputSlot;
			element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;

			inputElements.push_back(element);
			inputSlot++;
		}

		return inputElements;
	}

    void ShaderPipeline::UploadData(ID3D12GraphicsCommandList* commandList)
    {
		commandList->SetGraphicsRootSignature(m_rootSignature.Get());
		commandList->SetPipelineState(m_pipelineState.Get());
    }
}