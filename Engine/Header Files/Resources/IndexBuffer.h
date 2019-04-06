#pragma once
#include "GPUResource.h"

namespace Engine
{
    class IndexBuffer : public GPUResource
    {
    public:
        using Ptr = std::shared_ptr<IndexBuffer>;
        IndexBuffer(void* data, uint32_t indexCount, bool bigIndices = true);

        void UploadData(ID3D12GraphicsCommandList* commandList);

    private:
		D3D12_INDEX_BUFFER_VIEW m_IBV;
        const UINT m_indexSizeInBytes;
        const uint32_t m_indexCount;
        const DXGI_FORMAT m_indexFormat;
    };
}