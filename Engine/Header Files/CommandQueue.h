#pragma once
#include "pch.h"
#include <mutex>
#include <queue>

namespace Engine
{
	class CommandQueue
	{
	public:
		CommandQueue(D3D12_COMMAND_LIST_TYPE type);
		~CommandQueue();

		bool IsFenceComplete(uint64_t fenceValue);
		void InsertWait(uint64_t fenceValue);
		void InsertWaitForQueueFence(CommandQueue* otherQueue, uint64_t fenceValue);
		void InsertWaitForQueue(CommandQueue* otherQueue);

		void WaitForFenceCPUBlocking(uint64_t fenceValue);
		void WaitForIdle();

		ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }

		uint64_t PollCurrentFenceValue();
		uint64_t GetLastCompletedFence() const { return m_lastCompletedFenceValue; }
		uint64_t GetNextFenceValue() const { return m_nextFenceValue; }
		ID3D12Fence* GetFence() const { return m_fence.Get(); }

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
		uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	protected:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		D3D12_COMMAND_LIST_TYPE m_type;

		std::mutex m_fenceMutex;
		std::mutex m_eventMutex;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		uint64_t m_nextFenceValue;
		uint64_t m_lastCompletedFenceValue;
		HANDLE m_fenceEventHandle;

		// Keep track of command allocators that are "in-flight"
		struct CommandAllocatorEntry
		{
			uint64_t fenceValue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		};

		using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
		using CommandListQueue = std::queue< Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> >;
		CommandAllocatorQueue m_commandAllocatorQueue;
		CommandListQueue m_commandListQueue;
	};
}