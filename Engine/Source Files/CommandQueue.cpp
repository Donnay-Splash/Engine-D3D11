#include "pch.h"
#include "CommandQueue.h"
#include "d3dclass.h"

namespace Engine
{
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) :
		m_type(type)
	{
		m_lastCompletedFenceValue = ((uint64_t)m_type << 56);
		m_nextFenceValue = m_lastCompletedFenceValue + 1;

		auto device = D3DClass::Instance()->GetDevice();

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = m_type;
		desc.NodeMask = 0;
		Utils::DirectXHelpers::ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)));

		Utils::DirectXHelpers::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fence->Signal(m_lastCompletedFenceValue);

		m_fenceEventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		EngineAssert(m_fenceEventHandle != INVALID_HANDLE_VALUE);
	}

	CommandQueue::~CommandQueue()
	{
		CloseHandle(m_fenceEventHandle);
		m_fence->Release();
		m_fence = nullptr;

		m_commandQueue->Release();
		m_commandQueue = nullptr;
	}

	uint64_t CommandQueue::PollCurrentFenceValue()
	{
		m_lastCompletedFenceValue = std::max(m_lastCompletedFenceValue, m_fence->GetCompletedValue());
		return m_lastCompletedFenceValue;
	}

	bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
	{
		if (fenceValue > m_lastCompletedFenceValue)
		{
			PollCurrentFenceValue();
		}

		return fenceValue <= m_lastCompletedFenceValue;
	}

	void CommandQueue::InsertWait(uint64_t fenceValue)
	{
		m_commandQueue->Wait(m_fence.Get(), fenceValue);
	}

	void CommandQueue::InsertWaitForQueueFence(CommandQueue* otherQueue, uint64_t fenceValue)
	{
		m_commandQueue->Wait(otherQueue->GetFence(), fenceValue);
	}

	void CommandQueue::InsertWaitForQueue(CommandQueue* otherQueue)
	{
		m_commandQueue->Wait(otherQueue->GetFence(), otherQueue->GetNextFenceValue() - 1);
	}

	void CommandQueue::WaitForFenceCPUBlocking(uint64_t fenceValue)
	{
		if (IsFenceComplete(fenceValue))
		{
			return;
		}

		{
			std::lock_guard<std::mutex> lockGuard(m_eventMutex);

			m_fence->SetEventOnCompletion(fenceValue, m_fenceEventHandle);
			WaitForSingleObjectEx(m_fenceEventHandle, INFINITE, false);
			m_lastCompletedFenceValue = fenceValue;
		}
	}

	void CommandQueue::WaitForIdle()
	{
		WaitForFenceCPUBlocking(m_nextFenceValue - 1);
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandQueue::GetCommandList()
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

		if (!m_commandAllocatorQueue.empty() && IsFenceComplete(m_commandAllocatorQueue.front().fenceValue))
		{
			commandAllocator = m_commandAllocatorQueue.front().commandAllocator;
			m_commandAllocatorQueue.pop();

			Utils::DirectXHelpers::ThrowIfFailed(commandAllocator->Reset());
		}
		else
		{
			commandAllocator = CreateCommandAllocator();
		}

		if (!m_commandListQueue.empty())
		{
			commandList = m_commandListQueue.front();
			m_commandListQueue.pop();

			Utils::DirectXHelpers::ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
		}
		else
		{
			commandList = CreateCommandList(commandAllocator);
		}

		// Associate the command allocator with the command list so that it can be
		// retrieved when the command list is executed.
		Utils::DirectXHelpers::ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

		return commandList;
	}

	uint64_t CommandQueue::ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		Utils::DirectXHelpers::ThrowIfFailed(commandList->Close());

		ID3D12CommandAllocator* commandAllocator;
		UINT dataSize = sizeof(commandAllocator);
		Utils::DirectXHelpers::ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

		ID3D12CommandList* const ppCommandLists[] = {
			commandList.Get()
		};

		m_commandQueue->ExecuteCommandLists(1, ppCommandLists);
		{
			std::lock_guard<std::mutex> lockGuard(m_fenceMutex);
			m_commandQueue->Signal(m_fence.Get(), m_nextFenceValue);
		}

		m_commandAllocatorQueue.emplace(CommandAllocatorEntry{ m_nextFenceValue, commandAllocator });
		m_commandListQueue.push(commandList);

		// The ownership of the command allocator has been transferred to the ComPtr
		// in the command allocator queue. It is safe to release the reference 
		// in this temporary COM pointer here.
		commandAllocator->Release();

		return m_nextFenceValue++;
	}

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		Utils::DirectXHelpers::ThrowIfFailed(D3DClass::Instance()->GetDevice()->CreateCommandAllocator(m_type, IID_PPV_ARGS(&commandAllocator)));

		return commandAllocator;
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandQueue::CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
		Utils::DirectXHelpers::ThrowIfFailed(D3DClass::Instance()->GetDevice()->CreateCommandList(0, m_type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

		return commandList;
	}
}