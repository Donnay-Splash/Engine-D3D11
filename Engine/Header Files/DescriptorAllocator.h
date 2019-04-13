#pragma once
#include "pch.h"

namespace Engine
{
	struct DescriptorPair
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
	};

	class DescriptorAllocator
	{
	public:
		DescriptorAllocator(UINT descriptorCount, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		~DescriptorAllocator();

		DescriptorPair* AllocateDescriptor();
		void FreeDescriptor(DescriptorPair*);
		bool BelongsToHeap(DescriptorPair*);

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
		UINT m_descriptorSize;
		UINT m_descriptorCount;
		std::vector<std::unique_ptr<DescriptorPair>> m_descriptors;
		std::vector<DescriptorPair*> m_freeDescriptors;
	};
}