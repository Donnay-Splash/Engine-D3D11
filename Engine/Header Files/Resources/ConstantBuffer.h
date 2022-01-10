#pragma once
#include "ShaderResource.h"
#include "d3dclass.h"

namespace Engine
{
    namespace PipelineStage
    {
        static const uint32_t Vertex = 0x0001;
        static const uint32_t Pixel = 0x0002;
        static const uint32_t All = Vertex | Pixel;
    }

    template <class T>
    class ConstantBuffer : public ShaderResource, public GPUResource
    {
    public:
        using Ptr = std::shared_ptr<ConstantBuffer<T>>;

        ConstantBuffer(uint32_t pipelineStages = PipelineStage::All) : GPUResource(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        {
            // Create constant buffer
            auto device = D3DClass::Instance()->GetDevice();
            Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_resource)));

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = m_resource->GetGPUVirtualAddress();
            cbvDesc.SizeInBytes = (sizeof(T) + 255) & ~255; // Align up 256 bytes
            m_CBV = D3DClass::Instance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            device->CreateConstantBufferView(&cbvDesc, m_CBV->CPUHandle);

            // Map and initialize constant buffer
            CD3DX12_RANGE readRange(0, 0); // We're not reading this resources
            Utils::DirectXHelpers::ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&m_DataPtr)));

            // Constant buffer must be bound to atleast one pipeline stage
            // Note: Dunno if that is still useful for DX12
            EngineAssert(pipelineStages > 0);

				// TODO: Look at implementing some sort of memory allocator so that we don't have to double buffer constant buffers. Yuck.
			// Creating a constant buffer in DX12
			// First we will need a descriptor heap to assign all of our CBV_SRV_UAV descriptors, this can be created and managed by the device.
			// Create a heap for our CBV resource. This can be used for all constant buffers we can again create and manage this on the device level.
			// Create the CBV. In DX12 we do this via descriptor
			// When setting the data we need to map the heap that the buffer is on and write the data to it. We can keep the data mapped for as long as we want
			// Just need to make sure that we don't modify the space in memory once we have sent this data to the GPU. All CBV updates need to be synchronized.

			// If we want to abstract all of this from the code we need to find a way to reference a buffer and its space on the heap.
        }

        ~ConstantBuffer()
        {
            D3DClass::Instance()->FreeDescriptor(m_CBV);
        }

        // TODO Need to find better name for this 
        void SetData(T data)
        {
            // TODO: Find a better way to auto define comparison operators.
            // Also is it even worth adding?
            //if (m_data != data)
            {
                m_data = data;
                m_dataChanged = true;
            }
        }

        T GetData() const { return m_data; }

        void UploadData()
        {
            if (m_dataChanged)
            {
                // Copy the data into the constant buffer.
                memcpy(m_DataPtr, &m_data, sizeof(T));

                m_dataChanged = false;
            }

            /*if (m_pipelineStages & PipelineStage::Vertex)
            {
                deviceContext->VSSetConstantBuffers(T::kRegister, 1, m_buffer.GetAddressOf());
            }

            if (m_pipelineStages & PipelineStage::Pixel)
            {
                deviceContext->PSSetConstantBuffers(T::kRegister, 1, m_buffer.GetAddressOf());
            }*/
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() { return m_CBV->CPUHandle; }

        CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() { return m_CBV->GPUHandle; }

    private:
        __declspec(align(16))
            T m_data;
        bool m_dataChanged = true;
        DescriptorPair* m_CBV;
        uint8_t* m_DataPtr;
    };
}
