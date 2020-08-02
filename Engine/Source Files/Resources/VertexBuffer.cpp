#include "pch.h"
#include <Resources\VertexBuffer.h>
#include "d3dclass.h"

namespace Engine
{
	VertexBuffer::VertexBuffer(void* data, uint32_t vertexCount, size_t vertexSizeInBytes)
		: GPUResource(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER), m_stride(static_cast<UINT>(vertexSizeInBytes)), m_DynamicBuffer(data == nullptr)
	{
		// The vertex must have a size greater than zero
		EngineAssert(m_stride > 0);
		// Why are you making a vertex buffer with no vertices?
		EngineAssert(vertexCount > 0);

		m_BufferSize = vertexCount * vertexSizeInBytes;

		// When data is null we assume that we will be dynamically updating this buffer
		// If not null then it will be uploaded once and stay static for the rest of its lifetime
		D3D12_HEAP_TYPE heapType = m_DynamicBuffer ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
		D3D12_RESOURCE_STATES initialState = m_DynamicBuffer ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(heapType),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_BufferSize),
			initialState,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf())));

		if (data)
		{
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = data;
			subresourceData.RowPitch = m_BufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			D3DClass::Instance()->UploadSubresourceData(m_BufferSize, &subresourceData, m_resource.Get(), m_usageState);
		}

		m_VBV.BufferLocation = m_resource->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = (UINT)m_BufferSize;
		m_VBV.StrideInBytes = (UINT)vertexSizeInBytes;
    }

	void VertexBuffer::SetNewData(void* data, uint64_t dataSize)
	{
		// Only allow post creation data modification if it's a dynamic buffer
		EngineAssert(m_DynamicBuffer);
		EngineAssert(dataSize <= m_BufferSize);
		D3D12_RANGE range;
		// A range where the start and end are equal means the CPU won't read memory
		memset(&range, 0, sizeof(D3D12_RANGE));
		void* resourceStart;
		Utils::DirectXHelpers::ThrowIfFailed(m_resource->Map(0, &range, &resourceStart));
		memcpy(resourceStart, data, dataSize);
        m_resource->Unmap(0, &range);
	}

    void VertexBuffer::UploadData(ID3D12GraphicsCommandList* commandList, UINT inputSlot)
    {
        // Set the vertex buffer to active in the input assembler so it can be rendered.
		commandList->IASetVertexBuffers(inputSlot, 1, &m_VBV);
    }
}