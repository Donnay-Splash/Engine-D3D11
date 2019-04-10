#include "pch.h"
#include <Resources\IndexBuffer.h>
#include "d3dclass.h"

namespace Engine
{
    IndexBuffer::IndexBuffer(void* data, uint32_t indexCount, bool  useBigIndices/* = true*/)
        : GPUResource(D3D12_RESOURCE_STATE_INDEX_BUFFER),
		m_indexCount(indexCount),
        m_indexSizeInBytes(useBigIndices ? sizeof(uint32_t) : sizeof(uint16_t)),
        m_indexFormat(useBigIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT)
    {
		// The vertex must have a size greater than zero
		EngineAssert(m_indexCount > 0);
		// Why are you making a vertex buffer with no vertices?
		EngineAssert(m_indexSizeInBytes > 0);

		uint64_t bufferSize = m_indexCount * m_indexSizeInBytes;

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = data;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		D3DClass::Instance()->UploadSubresourceData(bufferSize, &subresourceData, m_resource.Get(), m_usageState);

		m_IBV.BufferLocation = m_resource->GetGPUVirtualAddress();
		m_IBV.SizeInBytes = (UINT)bufferSize;
		m_IBV.Format = m_indexFormat;
    }

    void IndexBuffer::UploadData(ID3D12GraphicsCommandList* commandList)
    {
        // Set the index buffer to active in the input assembler so it can be rendered.
        commandList->IASetIndexBuffer(&m_IBV);
    }
}