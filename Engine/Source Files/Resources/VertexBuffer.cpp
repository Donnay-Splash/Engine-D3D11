#include "pch.h"
#include <Resources\VertexBuffer.h>
#include "d3dclass.h"

namespace Engine
{
    VertexBuffer::VertexBuffer(void* data, uint32_t vertexCount, size_t vertexSizeInBytes)
        : GPUResource(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER), m_stride(static_cast<UINT>(vertexSizeInBytes))
    {
        // The vertex must have a size greater than zero
        EngineAssert(m_stride > 0);
        // Why are you making a vertex buffer with no vertices?
        EngineAssert(vertexCount > 0);

		uint64_t bufferSize = vertexCount * vertexSizeInBytes;

		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(m_resource.GetAddressOf())));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = data;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		D3DClass::Instance()->UploadSubresourceData(bufferSize, &subresourceData, m_resource.Get(), m_usageState);

		m_VBV.BufferLocation = m_resource->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = (UINT)bufferSize;
		m_VBV.StrideInBytes = (UINT)vertexSizeInBytes;
    }

    void VertexBuffer::UploadData(ID3D12GraphicsCommandList* commandList, UINT inputSlot)
    {
        // Set the vertex buffer to active in the input assembler so it can be rendered.
		commandList->IASetVertexBuffers(inputSlot, 1, &m_VBV);
    }
}