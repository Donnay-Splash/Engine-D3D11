#pragma once
#include "pch.h"
namespace Engine
{
	class GPUResource
	{
	public:
		using Ptr = std::shared_ptr<GPUResource>;
		GPUResource(D3D12_RESOURCE_STATES initialState) :
			m_usageState(initialState)
		{
			m_address = 0;
			m_isReady = false;
		}

		virtual ~GPUResource() 
		{
			m_resource = nullptr;
		}

		ID3D12Resource* GetResource() const { return m_resource.Get(); }
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_address; }
		D3D12_RESOURCE_STATES GetUsageState() const { return m_usageState; }
		bool GetIsReady() const { return m_isReady; }

		void SetUsageState(D3D12_RESOURCE_STATES state) { m_usageState = state; }
		void SetIsReady(bool isReady) { m_isReady = isReady; }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
		D3D12_GPU_VIRTUAL_ADDRESS m_address;
		D3D12_RESOURCE_STATES m_usageState;
		bool m_isReady;
	};
}