#pragma once
#include "GPUResource.h"

namespace Engine
{
    class IndexBuffer : public GPUResource
    {
    public:
        using Ptr = std::shared_ptr<IndexBuffer>;
        IndexBuffer(void* data, uint64_t indexCount, bool bigIndices = true);

        void UploadData(ID3D12GraphicsCommandList* commandList);
        void SetNewData(void* data, uint64_t indexCount);
        uint64_t GetIndexCount() { return m_indexCount; }

    private:
		D3D12_INDEX_BUFFER_VIEW m_IBV;
        const UINT m_indexSizeInBytes;
        const uint64_t m_indexCount;
        const DXGI_FORMAT m_indexFormat;

        // TODO: Maybe we can put this on a GPUResource level?
        bool m_DynamicBuffer;
    };
}