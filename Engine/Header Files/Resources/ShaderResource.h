#pragma once
#include "GPUResource.h"
#include "DescriptorAllocator.h"

namespace Engine
{
	// Basically a GPU resource except it has descriptors;
	class ShaderResource
	{
	public:
		ShaderResource() {}
		virtual ~ShaderResource();

		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { EngineAssert(m_descriptor); return m_descriptor->CPUHandle; }
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { EngineAssert(m_descriptor); return m_descriptor->GPUHandle; }

	protected:
		DescriptorPair* m_descriptor = nullptr;
	};
}