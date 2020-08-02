#pragma once
#include "GPUResource.h"

namespace Engine
{
    class VertexBuffer : public GPUResource
    {
    public:
        using Ptr = std::shared_ptr<VertexBuffer>;
        VertexBuffer(void* data, uint32_t vertexCount, size_t vertexSizeInBytes);

        void UploadData(ID3D12GraphicsCommandList* commandList, UINT inputSlot);
        void SetNewData(void* data, uint64_t dataSize);

        uint64_t GetBufferSize() { return m_BufferSize; }
    private:
		D3D12_VERTEX_BUFFER_VIEW m_VBV;
        UINT m_stride;
        uint64_t m_BufferSize;

        bool m_DynamicBuffer;
    };
}