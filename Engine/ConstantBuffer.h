#pragma once
#include "pch.h"

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
    ConstantBuffer(uint32_t pipelineStages, ID3D11Device* device) :
        m_pipelineStages(pipelineStages)
    {
        // Constant buffer must be bound to atleast one pipeline stage
        EngineAssert(pipelineStages > 0);
        // Constant buffer creation should be kept to no more than two lines.
        // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
        D3D11_BUFFER_DESC matrixBufferDesc;
        matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        matrixBufferDesc.ByteWidth = sizeof(T);
        matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        matrixBufferDesc.MiscFlags = 0;
        matrixBufferDesc.StructureByteStride = 0;

        // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
        Utils::DirectXHelpers::ThrowIfFailed(device->CreateBuffer(&matrixBufferDesc, NULL, &m_buffer));
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
