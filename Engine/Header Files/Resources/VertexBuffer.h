#pragma once


namespace Engine
{
    class VertexBuffer
    {
    public:
        using Ptr = std::shared_ptr<VertexBuffer>;
        VertexBuffer(void* data, uint32_t vertexCount, size_t vertexSizeInBytes);

        void UploadData(ID3D11DeviceContext* deviceContext, UINT inputSlot, UINT offset);
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
        UINT m_stride;
    };
}