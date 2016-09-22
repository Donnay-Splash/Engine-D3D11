#include "pch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(void* data, uint32_t indexCount, ID3D11Device* device, bool  useBigIndices/* = true*/)
	: m_indexCount(indexCount), 
	m_indexSizeInBytes(useBigIndices ? sizeof(uint32_t) : sizeof(uint16_t)),
	m_indexFormat(useBigIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT)
{
	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
    SecureZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = m_indexSizeInBytes * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = data;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	Utils::DirectXHelpers::ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &indexData, m_buffer.GetAddressOf()));
}

void IndexBuffer::UploadData(ID3D11DeviceContext* deviceContext)
{
	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_buffer.Get(), m_indexFormat, 0);
}