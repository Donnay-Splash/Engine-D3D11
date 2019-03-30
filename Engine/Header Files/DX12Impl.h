#pragma once

//////////////
// INCLUDES //
//////////////
#include "d3dclass.h"

namespace Engine
{
	// Forward Declarations
	struct EngineCreateOptions;

	class DX12Impl : public D3DClass
	{
	public:
		using Ptr = std::shared_ptr<DX12Impl>;
		DX12Impl();
		DX12Impl(const DX12Impl&);
		virtual ~DX12Impl();

		void Shutdown() override;

		void ResizeBuffers(uint32_t newWidth, uint32_t newHeight) override;

		void BeginScene(float, float, float, float) override;
		void EndScene() override;

		// Begin and end render event must be called in pairs.
		// They can be nested bu there must be a matching end for every begin
		void BeginRenderEvent(const std::wstring& eventLabel) override;
		void EndRenderEvent() override;

		// Potentially rename FrameBuffer?
		void SetRenderTarget(RenderTargetBundle::Ptr, Utils::Maths::Vector2 clipOffset = {}) const override;

		void GetVideoCardInfo(char*, int&) override;

		Utils::Maths::Vector2 GetScreenSize() const override;

		Texture::Ptr CopyTexture(Texture::Ptr textureToCopy) const override;

		Texture::Ptr CopyBackBuffer() const override;

		void UnbindAllRenderTargets() const override;
		void UnbindShaderResourceView(uint32_t slot) const override;

	protected:
		void Initialize_Internal(EngineCreateOptions createOptions) override;

	private:
		void GetAdapterInformation();
		void CreateDeviceResourcesAndSwapChain();
		void CreateDescriptorHeaps();
		void CreateCommandList();
		void CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight);
		void CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight);
		void CreateBackBufferResources();
		void ClearResources();

		void WaitForPreviousFrame();

	private:
		bool m_vsync_enabled;
		int m_videoCardMemory;
		std::string m_videoCardDescription;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D12Device5> m_device;
		Microsoft::WRL::ComPtr<ID3D12DebugDevice2> m_debugDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		UINT m_rtvDescriptorSize;
		//Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
		//Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_userDefinedAnnotation;
		Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;

		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;

		// TODO: Eventually put this back into a DX12 compatible class
		static const UINT kBufferCount = 2;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[kBufferCount];

		//RenderTarget::Ptr m_backBufferRT;
		DepthBuffer::Ptr m_depthBuffer;
		Utils::Maths::Vector2 m_screenSize;

		// Use a single back buffer for now.
		EngineCreateOptions m_createOptions;
	};
}
