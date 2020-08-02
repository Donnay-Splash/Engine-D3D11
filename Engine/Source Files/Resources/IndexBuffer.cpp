#include "pch.h"
#include <Resources\IndexBuffer.h>
#include "d3dclass.h"

namespace Engine
{
    IndexBuffer::IndexBuffer(void* data, uint64_t indexCount, bool  useBigIndices/* = true*/)
        : GPUResource(D3D12_RESOURCE_STATE_INDEX_BUFFER),
		m_indexCount(indexCount),
        m_indexSizeInBytes(useBigIndices ? sizeof(uint32_t) : sizeof(uint16_t)),
        m_indexFormat(useBigIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT),
		m_DynamicBuffer(data == nullptr)
    {
		// The vertex must have a size greater than zero
		EngineAssert(m_indexCount > 0);
		// Why are you making a vertex buffer with no vertices?
		EngineAssert(m_indexSizeInBytes > 0);

		uint64_t bufferSize = m_indexCount * (uint64_t)m_indexSizeInBytes;

        // When data is null we assume that we will be dynamically updating this buffer
        // If not null then it will be uploaded once and stay static for the rest of its lifetime
        D3D12_HEAP_TYPE heapType = m_DynamicBuffer ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_STATES initialState = m_DynamicBuffer ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(heapType),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			initialState,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf())));

		if (data)
		{
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = data;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			D3DClass::Instance()->UploadSubresourceData(bufferSize, &subresourceData, m_resource.Get(), m_usageState);
		}

		m_IBV.BufferLocation = m_resource->GetGPUVirtualAddress();
		m_IBV.SizeInBytes = (UINT)bufferSize;
		m_IBV.Format = m_indexFormat;
    }

	void IndexBuffer::SetNewData(void* data, uint64_t indexCount)
	{
        // Only allow post creation data modification if it's a dynamic buffer
        EngineAssert(m_DynamicBuffer);
        EngineAssert(indexCount <= m_indexCount);
        D3D12_RANGE range;
        // A range where the start and end are equal means the CPU won't read memory
        memset(&range, 0, sizeof(D3D12_RANGE));
        void* resourceStart;
        Utils::DirectXHelpers::ThrowIfFailed(m_resource->Map(0, &range, &resourceStart));
        memcpy(resourceStart, data, indexCount * m_indexSizeInBytes);
        m_resource->Unmap(0, &range);
	}

    void IndexBuffer::UploadData(ID3D12GraphicsCommandList* commandList)
    {
        // Set the index buffer to active in the input assembler so it can be rendered.
        commandList->IASetIndexBuffer(&m_IBV);
    }
}