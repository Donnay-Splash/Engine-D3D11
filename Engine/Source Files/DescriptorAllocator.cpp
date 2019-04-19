#include "pch.h"
#include "DescriptorAllocator.h"
#include "d3dclass.h"

namespace Engine
{
	DescriptorAllocator::DescriptorAllocator(UINT descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags /*= D3D12_DESCRIPTOR_HEAP_FLAG_NONE*/):
		m_descriptorCount(descriptorCount)
	{
		ID3D12Device* device = D3DClass::Instance()->GetDevice();
		m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = m_descriptorCount;
		desc.Type = type;
		desc.Flags = flags;

		Utils::DirectXHelpers::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptorHeap)));

		m_descriptors.resize(m_descriptorCount);
		m_freeDescriptors.resize(m_descriptorCount);

		// Fill free descriptors list.
		for (UINT i = 0; i < m_descriptorCount; ++i)
		{
			m_descriptors[i] = std::make_unique<DescriptorPair>();
			m_freeDescriptors[i] = m_descriptors[i].get();

			m_descriptors[i]->CPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), i, m_descriptorSize);
			m_descriptors[i]->GPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), i, m_descriptorSize);
		}
	}

	DescriptorPair* DescriptorAllocator::AllocateDescriptor()
	{
		EngineAssert(!m_freeDescriptors.empty());
		DescriptorPair* descriptor = m_freeDescriptors.front();
		m_freeDescriptors.erase(m_freeDescriptors.begin());

		return descriptor;
	}

	void DescriptorAllocator::FreeDescriptor(DescriptorPair* descriptor)
	{
		m_freeDescriptors.push_back(descriptor);
	}

	bool DescriptorAllocator::BelongsToHeap(DescriptorPair* descriptor)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE begin(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE end(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_descriptorCount, m_descriptorSize);
		return (descriptor->CPUHandle.ptr >= begin.ptr) && (descriptor->CPUHandle.ptr < end.ptr);
	}
}