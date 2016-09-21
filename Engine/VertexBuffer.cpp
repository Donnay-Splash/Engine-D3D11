#include "pch.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(void* data, uint32_t vertexCount, size_t vertexSizeInBytes, ID3D11Device* device)
	: m_stride(static_cast<UINT>(vertexSizeInBytes))
{
	// The vertex must have a size greater than zero
	EngineAssert(m_stride > 0);
	// Why are you making a vertex buffer with no vertices?
	EngineAssert(vertexCount > 0);

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = m_stride * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = data;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	Utils::DirectXHelpers::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexData, m_buffer.GetAddressOf()));
}

void VertexBuffer::UploadData(ID3D11DeviceContext* deviceContext, UINT inputSlot, UINT offset)
{
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(inputSlot, 1, m_buffer.GetAddressOf(), &m_stride, &offset);
}