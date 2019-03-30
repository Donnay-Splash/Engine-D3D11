#pragma once

namespace Engine
{
    namespace PipelineStage
    {
        static const uint32_t Vertex = 0x0001;
        static const uint32_t Pixel = 0x0002;
    }

    template <class T>
    class ConstantBuffer
    {
    public:
        using Ptr = std::shared_ptr<ConstantBuffer<T>>;
        ConstantBuffer(uint32_t pipelineStages) :
            m_pipelineStages(pipelineStages)
        {
            // Constant buffer must be bound to atleast one pipeline stage
            EngineAssert(pipelineStages > 0);
            // Constant buffer creation should be kept to no more than two lines.
            // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
            D3D11_BUFFER_DESC matrixBufferDesc;
            SecureZeroMemory(&matrixBufferDesc, sizeof(matrixBufferDesc));
            matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            matrixBufferDesc.ByteWidth = sizeof(T);
            matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            matrixBufferDesc.MiscFlags = 0;
            matrixBufferDesc.StructureByteStride = 0;

            // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
			IMPLEMENT_FOR_DX12(Utils::DirectXHelpers::ThrowIfFailed(device->CreateBuffer(&matrixBufferDesc, NULL, &m_buffer));)

			// Creating a constant buffer in DX12
			// First we will need a descriptor heap to assign all of our CBV_SRV_UAV descriptors, this can be created and managed by the device.
			// Create a heap for our CBV resource. This can be used for all constant buffers we can again create and manage this on the device level.
			// Create the CBV. In DX12 we do this via descriptor
			// When setting the data we need to map the heap that the buffer is on and write the data to it. We can keep the data mapped for as long as we want
			// Just need to make sure that we don't modify the space in memory once we have sent this data to the GPU. All CBV updates need to be synchronized.

			// If we want to abstract all of this from the code we need to find a way to reference a buffer and its space on the heap.
        }

        // TODO Need to find better name for this 
        void SetData(T data)
        {
            if (m_data != data)
            {
                m_data = data;
                m_dataChanged = true;
            }
        }

        T GetData() const { return m_data; }

        void UploadData(ID3D11DeviceContext* deviceContext)
        {
            if (m_dataChanged)
            {
                D3D11_MAPPED_SUBRESOURCE mappedResource;
                // Lock the constant buffer so it can be written to.
                deviceContext->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

                // Get a pointer to the data in the constant buffer.
                auto dataPtr = (T*)mappedResource.pData;

                // Copy the data into the constant buffer.
                memcpy(dataPtr, &m_data, sizeof(T));

                // Unlock the constant buffer.
                deviceContext->Unmap(m_buffer.Get(), 0);

                m_dataChanged = false;
            }

            if (m_pipelineStages & PipelineStage::Vertex)
            {
                deviceContext->VSSetConstantBuffers(T::kRegister, 1, m_buffer.GetAddressOf());
            }

            if (m_pipelineStages & PipelineStage::Pixel)
            {
                deviceContext->PSSetConstantBuffers(T::kRegister, 1, m_buffer.GetAddressOf());
            }
        }

    private:
        __declspec(align(16))
            T m_data;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
        const uint32_t m_pipelineStages;
        bool m_dataChanged = true;
    };
}
