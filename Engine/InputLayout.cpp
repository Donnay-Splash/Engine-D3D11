#include "pch.h"
#include "InputLayout.h"

InputLayout::InputLayout(const uint32_t& inputFlags)
{
	EngineAssert(inputFlags != 0);

    UINT inputSlot = 0;
	if (inputFlags & InputElement::Position)
	{
		D3D11_INPUT_ELEMENT_DESC element;
		element.SemanticName = "POSITION";
		element.SemanticIndex = 0;
		element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		element.InputSlot = inputSlot;
		element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;

		m_inputElements.push_back(element);
        inputSlot++;
	}

	if (inputFlags & InputElement::Normal0)
	{
		D3D11_INPUT_ELEMENT_DESC element;
		element.SemanticName = "NORMAL";
		element.SemanticIndex = 0;
		element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		element.InputSlot = inputSlot;
		element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;

		m_inputElements.push_back(element);
        inputSlot++;
	}

	if (inputFlags & InputElement::TexCoord0)
	{
		D3D11_INPUT_ELEMENT_DESC element;
		element.SemanticName = "TEXCOORD";
		element.SemanticIndex = 0;
		element.Format = DXGI_FORMAT_R32G32_FLOAT;
		element.InputSlot = inputSlot;
		element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element.InstanceDataStepRate = 0;

		m_inputElements.push_back(element);
        inputSlot++;
	}

	// The input cannot be emptty.
	EngineAssert(!m_inputElements.empty());
}

void InputLayout::UploadData(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetInputLayout(m_inputLayout.Get());
}