#pragma once

//////////////
// INCLUDES //
//////////////
#include "EngineCreateOptions.h"
#include "RenderTargetBundle.h"
#include "CommandQueue.h"
#include "DescriptorAllocator.h"
#include "Resources\RenderTarget.h"

namespace Engine
{
    // Forward Declarations
    struct EngineCreateOptions;

    class D3DClass
    {
    public:
        using Ptr = std::shared_ptr<D3DClass>;
		D3DClass();
		D3DClass(const D3DClass&);
		virtual ~D3DClass();

        static void Initialize(EngineCreateOptions createOptions);
		static D3DClass* Instance();
        void Shutdown();

        void ResizeBuffers(uint32_t newWidth, uint32_t newHeight);

		ID3D12Device4* GetDevice() const { return m_device.Get(); }

        void BeginScene(float, float, float, float);
        void EndScene();

        // Begin and end render event must be called in pairs.
        // They can be nested bu there must be a matching end for every begin
        void BeginRenderEvent(const std::wstring& eventLabel);
        void EndRenderEvent();

		Utils::Maths::Vector2 GetScreenSize() const;

        // Potentially rename FrameBuffer?
		void SetRenderTarget(RenderTargetBundle::Ptr, Utils::Maths::Vector2 clipOffset = {}) const;

		void GetVideoCardInfo(char*, int&);

		Texture::Ptr CopyTexture(Texture::Ptr textureToCopy) const;

		Texture::Ptr CopyBackBuffer() const;

		void UnbindAllRenderTargets() const;
		void UnbindShaderResourceView(uint32_t slot) const;

		void SetShaderVisibleDescriptorHeaps();

		// Probably a better place to eventually put this.
		void UploadSubresourceData(uint64_t bufferSize, D3D12_SUBRESOURCE_DATA* data, ID3D12Resource* destinationResource, D3D12_RESOURCE_STATES intendedState);

		ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

		CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const { return m_commandQueues.at(type).get(); }
		bool IsFenceComplete(uint64_t fenceValue) const { return GetCommandQueue(D3D12_COMMAND_LIST_TYPE(fenceValue >> 56))->IsFenceComplete(fenceValue); }
		void WaitForFenceCPUBlocking(uint64_t fenceValue) { return GetCommandQueue(D3D12_COMMAND_LIST_TYPE(fenceValue >> 56))->WaitForFenceCPUBlocking(fenceValue); }
		void WaitForIdle() { for (auto &commandQueue : m_commandQueues) { commandQueue.second->WaitForIdle(); } }

		DescriptorPair* AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type) { return m_DescriptorHeaps[type]->AllocateDescriptor(); }
		void FreeDescriptor(DescriptorPair* descriptor);

	private:
		void Initialize_Internal(EngineCreateOptions createOptions);
		void GetAdapterInformation();
		void CreateDeviceResourcesAndSwapChain();
		void CreateDescriptorHeaps();
		void CreateCommandList();
		void CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight);
		void CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight);
		void CreateBackBufferResources(uint32_t screenWidth, uint32_t screenHeight);
		void ClearResources();

		void WaitForPreviousFrame();

	private:
		bool m_vsync_enabled;
		int m_videoCardMemory;
		std::string m_videoCardDescription;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12Device4> m_device;
		Microsoft::WRL::ComPtr<ID3D12DebugDevice1> m_debugDevice;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		//Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_userDefinedAnnotation;
		Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;

		D3D12_VIEWPORT m_viewport;
		RECT m_scissor;

		// Store all of our command queues
		std::map<D3D12_COMMAND_LIST_TYPE, std::unique_ptr<CommandQueue>> m_commandQueues;

		// TODO: Put these in some sort of manager class to simplify things
		// Store all descriptor heaps.
		std::map<D3D12_DESCRIPTOR_HEAP_TYPE, std::unique_ptr<DescriptorAllocator>> m_DescriptorHeaps;

		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;

		// TODO: Eventually put this back into a DX12 compatible class
		static const UINT kBufferCount = 2;
		std::unique_ptr<RenderTarget> m_renderTargets[kBufferCount];

		//RenderTarget::Ptr m_backBufferRT;
		DepthBuffer::Ptr m_depthBuffer;
		Utils::Maths::Vector2 m_screenSize;

		// Use a single back buffer for now.
		EngineCreateOptions m_createOptions;
	};
}
