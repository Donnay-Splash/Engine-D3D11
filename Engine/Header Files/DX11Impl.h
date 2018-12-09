#pragma once

//////////////
// INCLUDES //
//////////////
#include "d3dclass.h"

namespace Engine
{
	// Forward Declarations
	struct EngineCreateOptions;

	class DX11Impl : public D3DClass
	{
	public:
		using Ptr = std::shared_ptr<DX11Impl>;
		DX11Impl();
		DX11Impl(const DX11Impl&);
		virtual ~DX11Impl();

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

		// TODO: Need to get rid of these as they directly expose underlying device types
		ID3D11Device* GetDevice();
		ID3D11DeviceContext* GetDeviceContext();

		void GetVideoCardInfo(char*, int&) override;

		Utils::Maths::Vector2 GetScreenSize() const;

		Texture::Ptr CopyTexture(Texture::Ptr textureToCopy) const override;

		Texture::Ptr CopyBackBuffer() const override;

		void UnbindAllRenderTargets() const override;
		void UnbindShaderResourceView(uint32_t slot) const override;

	protected:
		void Initialize_Internal(EngineCreateOptions createOptions) override;

	private:
		void GetAdapterInformation();
		void CreateDeviceAndSwapChain();
		void CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight);
		void CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight);

		void CreateBackBufferResources(ID3D11Texture2D* backbufferPtr);

		void ClearResources();

	private:
		bool m_vsync_enabled;
		int m_videoCardMemory;
		std::string m_videoCardDescription;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11Device1> m_device;
		Microsoft::WRL::ComPtr<ID3D11Debug> m_debugDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_userDefinedAnnotation;
		Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> m_adapter;
		RenderTarget::Ptr m_backBufferRT;
		DepthBuffer::Ptr m_depthBuffer;
		Utils::Maths::Vector2 m_screenSize;

		// Use a single back buffer for now.
		static const UINT kBufferCount = 2;
		EngineCreateOptions m_createOptions;
	};
}
