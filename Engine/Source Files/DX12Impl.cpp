//////////////////////////////////////////////////////////////////////////////////
//// Filename: DX12Impl.cpp
//////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
//#include "DX12Impl.h"
//#include "Engine.h"
//
//namespace Engine
//{
//	static std::string temp;
//	DX12Impl::DX12Impl()
//	{
//		m_swapChain = nullptr;
//		m_device = nullptr;
//		//m_deviceContext = nullptr;
//	}
//
//
//	DX12Impl::DX12Impl(const DX12Impl& other)
//	{
//	}
//
//
//	DX12Impl::~DX12Impl()
//	{
//		//m_deviceContext->ClearState();
//		m_debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_FLAGS::D3D12_RLDO_DETAIL);
//	}
//
//
//	void DX12Impl::Initialize_Internal(EngineCreateOptions createOptions)
//	{
//		// Store the vsync setting.
//		m_createOptions = createOptions;
//		m_vsync_enabled = m_createOptions.VSyncEnabled;
//
//		EngineAssert(m_createOptions.ScreenWidth > 0);
//		EngineAssert(m_createOptions.ScreenHeight > 0);
//		m_screenSize = { static_cast<float>(m_createOptions.ScreenWidth), static_cast<float>(m_createOptions.ScreenHeight) };
//
//		UINT dxgiFactoryFlags = 0;
//#if defined(_DEBUG)
//		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
//		Utils::DirectXHelpers::ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
//		debugController->EnableDebugLayer();
//
//		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
//#endif
//
//		// Create a DirectX graphics interface factory.
//		Utils::DirectXHelpers::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));
//
//		// Retrieve information from the adapter.
//		GetAdapterInformation();
//
//		// Create the device and swap chain
//		CreateDeviceResourcesAndSwapChain();
//	}
//
//	void DX12Impl::GetAdapterInformation()
//	{
//		Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
//		DXGI_ADAPTER_DESC adapterDesc;
//
//		// Use the factory to create an adapter for the primary graphics interface (video card).
//		Utils::DirectXHelpers::ThrowIfFailed(m_factory->EnumAdapters(0, m_adapter.GetAddressOf()));
//
//		// Get the adapter (video card) description.
//		Utils::DirectXHelpers::ThrowIfFailed(m_adapter->GetDesc(&adapterDesc));
//
//		// Store the dedicated video card memory in megabytes.
//		m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
//
//		// Convert the name of the video card from wide to standard character length.
//		std::wstring wideAdapterDescription(adapterDesc.Description);
//		m_videoCardDescription = std::string(wideAdapterDescription.begin(), wideAdapterDescription.end());
//	}
//
//	void DX12Impl::CreateDeviceResourcesAndSwapChain()
//	{
//		// Set the feature level to DirectX 11.
//		D3D_FEATURE_LEVEL featureLevelsSupported[] =
//		{
//			D3D_FEATURE_LEVEL_12_1,
//			D3D_FEATURE_LEVEL_12_0,
//			D3D_FEATURE_LEVEL_11_1,
//			D3D_FEATURE_LEVEL_11_0,
//			D3D_FEATURE_LEVEL_10_1,
//			D3D_FEATURE_LEVEL_10_0,
//		};
//
//		Utils::DirectXHelpers::ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
//		// Query the debug device from the device
//		Utils::DirectXHelpers::ThrowIfFailed(m_device.As(&m_debugDevice));
//
//		// Create the command queue, allocator and list
//		CreateCommandList();
//
//		// Now create all of our global descriptor heaps.
//		CreateDescriptorHeaps();
//
//		switch (m_createOptions.RendererMode)
//		{
//		case EngineRendererMode::Win32:
//			CreateSwapChain_HWND(m_createOptions.ScreenWidth, m_createOptions.ScreenHeight);
//			break;
//		case EngineRendererMode::XAML:
//			CreateSwapChain_XAML(m_createOptions.ScreenWidth, m_createOptions.ScreenHeight);
//			break;
//		default:
//			// Incorrect enum value passed in
//			EngineAssert(false);
//			break;
//		}
//
//		// Now call swapchain created callback.
//		if (m_createOptions.SwapChainCreatedCallback != nullptr)
//		{
//			m_createOptions.SwapChainCreatedCallback(m_swapChain.Get(), m_createOptions.UserData);
//		}
//	}
//
//	void DX12Impl::CreateDescriptorHeaps()
//	{
//		// Will need to extend the count of descriptors for allocation of other render targets.
//		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
//		rtvHeapDesc.NumDescriptors = kBufferCount;
//		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
//
//		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//	}
//
//	void DX12Impl::CreateCommandList()
//	{
//		// Create the command queue
//		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
//		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//
//		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
//		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
//		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
//
//		// Command lists are created in the recording state, Close it now and we'll open it when we start recording commands
//		Utils::DirectXHelpers::ThrowIfFailed(m_commandList->Close());
//
//		// Create synchronisation objects
//		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
//		m_fenceValue = 1;
//
//		// Create event handle for frame synchronization
//		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
//		if (m_fenceEvent == nullptr)
//		{
//			Utils::DirectXHelpers::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
//		}
//	}
//
//	void DX12Impl::CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight)
//	{
//		if (m_swapChain == nullptr)
//		{
//			DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
//			SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
//
//			// Set to a single back buffer.
//			swapChainDesc.BufferCount = kBufferCount;
//			swapChainDesc.Width = screenWidth;
//			swapChainDesc.Height = screenHeight;
//			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//
//			// Set the usage of the back buffer.
//			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//
//			// Turn multisampling off.
//			swapChainDesc.SampleDesc.Count = 1;
//			swapChainDesc.SampleDesc.Quality = 0;
//
//			// Discard the back buffer contents after presenting.
//			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//
//			// Don't set the advanced flags.
//			swapChainDesc.Flags = 0;
//
//			// Create the swap chain
//			Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
//			auto hr = m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), m_createOptions.HWND, &swapChainDesc, NULL, NULL, tempSwapChain.GetAddressOf());
//			Utils::DirectXHelpers::ThrowIfFailed(hr);
//
//			// Convert to SwapChain3
//			Utils::DirectXHelpers::ThrowIfFailed(tempSwapChain.As(&m_swapChain));
//		}
//		else
//		{
//			// Resize swap chain
//			// TODO: Need to remember to release back buffers.
//			m_swapChain->ResizeBuffers(kBufferCount, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
//		}
//
//		CreateBackBufferResources();
//	}
//
//	void DX12Impl::CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight)
//	{
//		if (m_swapChain == nullptr)
//		{
//			//DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
//			//SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
//
//			//// Set to a single back buffer.
//			//swapChainDesc.BufferCount = kBufferCount;
//			//swapChainDesc.Width = screenWidth;
//			//swapChainDesc.Height = screenHeight;
//			//swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//
//			//// Set the usage of the back buffer.
//			//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//
//			//// Turn multisampling off.
//			//swapChainDesc.SampleDesc.Count = 1;
//			//swapChainDesc.SampleDesc.Quality = 0;
//
//			//// Discard the back buffer contents after presenting.
//			//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//
//			//// Don't set the advanced flags.
//			//swapChainDesc.Flags = 0;
//
//			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
//
//			swapChainDesc.Width = lround(screenWidth);		// Match the size of the window.
//			swapChainDesc.Height = lround(screenHeight);
//			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// This is the most common swap chain format.
//			swapChainDesc.Stereo = false;
//			swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
//			swapChainDesc.SampleDesc.Quality = 0;
//			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//			swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
//			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Windows Store apps must use _FLIP_ SwapEffects.
//			swapChainDesc.Flags = 0;
//			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
//			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
//
//			// Create the swap chain
//			Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
//			Utils::DirectXHelpers::ThrowIfFailed(m_factory->CreateSwapChainForComposition(m_commandQueue.Get(), &swapChainDesc, nullptr, tempSwapChain.GetAddressOf()));
//
//			// Convert to SwapChain3
//			Utils::DirectXHelpers::ThrowIfFailed(tempSwapChain.As(&m_swapChain));
//		}
//		else
//		{
//			// Resize swap chain
//			// TODO: Need to remember to release back buffers.
//			auto hr = (m_swapChain->ResizeBuffers(kBufferCount, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
//			if (FAILED(hr))
//			{
//				hr = m_device->GetDeviceRemovedReason();
//				Utils::DirectXHelpers::ThrowIfFailed(hr);
//			}
//		}
//
//		CreateBackBufferResources();
//	}
//
//	void DX12Impl::CreateBackBufferResources()
//	{
//		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
//
//		// Create an RTV for each frame
//		for (int n = 0; n < kBufferCount; ++n)
//		{
//			Utils::DirectXHelpers::ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
//			std::wstring name = L"BackBuffer:" + std::to_wstring(n);
//			m_renderTargets[n]->SetName(name.c_str());
//			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
//			rtvHandle.Offset(1, m_rtvDescriptorSize);
//		}
//
//		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
//
//
//		// Eventually come back to this
//
//		//auto backBufferWidth = m_backBufferRT->GetWidth();
//		//auto backBufferHeight = m_backBufferRT->GetHeight();
//		//m_screenSize = { float(backBufferWidth), float(backBufferHeight) };
//
//		//m_depthBuffer = std::make_shared<DepthBuffer>(backBufferWidth, backBufferHeight, 1, 0, m_device.Get());
//
//		//// Setup the viewport for rendering.
//		//D3D11_VIEWPORT viewport;
//		//viewport.Width = (float)backBufferWidth;
//		//viewport.Height = (float)backBufferHeight;
//		//viewport.MinDepth = 0.0f;
//		//viewport.MaxDepth = 1.0f;
//		//viewport.TopLeftX = 0.0f;
//		//viewport.TopLeftY = 0.0f;
//
//		//// Create the viewport.
//		//m_deviceContext->RSSetViewports(1, &viewport);
//	}
//
//	void DX12Impl::ClearResources()
//	{
//		for (int n = 0; n < kBufferCount; ++n)
//		{
//			m_renderTargets[n] = nullptr;
//		}
//
//		/*m_backBufferRT = nullptr;
//		m_depthBuffer = nullptr;
//
//		m_deviceContext->Flush();*/
//	}
//
//	void DX12Impl::Shutdown()
//	{
//		WaitForPreviousFrame();
//
//		// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
//		if (m_swapChain)
//		{
//			m_swapChain->SetFullscreenState(false, NULL);
//		}
//
//		//m_deviceContext = nullptr;
//		m_device = nullptr;
//		m_swapChain = nullptr;
//		CloseHandle(m_fenceEvent);
//	}
//
//	void DX12Impl::ResizeBuffers(uint32_t newWidth, uint32_t newHeight)
//	{
//		// TODO: Come up with way to stop unnecessary resize
//		//if (newWidth != m_backBufferRT->GetWidth() || newHeight != m_backBufferRT->GetHeight())
//		{
//			ClearResources();
//			// Resize buffers
//			switch (m_createOptions.RendererMode)
//			{
//			case EngineRendererMode::Win32:
//				CreateSwapChain_HWND(newWidth, newHeight);
//				break;
//			case EngineRendererMode::XAML:
//				CreateSwapChain_XAML(newWidth, newHeight);
//				break;
//			default:
//				// Incorrect enum value passed in
//				EngineAssert(false);
//				break;
//			}
//
//		}
//		// otherwise continue as normal
//	}
//
//
//	void DX12Impl::BeginScene(float red, float green, float blue, float alpha)
//	{
//		temp = "begin";
//		// Command list allocators can only be reset when the associated 
//		// command lists have finished execution on the GPU; apps should use 
//		// fences to determine GPU execution progress.
//		Utils::DirectXHelpers::ThrowIfFailed(m_commandAllocator->Reset());
//
//		// However, when ExecuteCommandList() is called on a particular command 
//		// list, that command list can then be reset at any time and must be before 
//		// re-recording.
//		Utils::DirectXHelpers::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
//
//		// Transition the back buffer back to render target
//		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
//
//		float color[4];
//		// Setup the color to clear the buffer to.
//		color[0] = red;
//		color[1] = green;
//		color[2] = blue;
//		color[3] = alpha;
//
//		// Clear the back buffer.
//		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
//		m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
//
//		// Clear the depth buffer.
//		//m_commandList->ClearDepthStencilView(m_depthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//
//		return;
//	}
//
//
//	void DX12Impl::EndScene()
//	{
//		// Transition the back-buffer to present 
//		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
//
//		// Close our command list
//		Utils::DirectXHelpers::ThrowIfFailed(m_commandList->Close());
//
//		// Execute our command list
//		ID3D12CommandList* commandLists[] = { m_commandList.Get() };
//		m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
//
//		// Present the back buffer to the screen since rendering is complete.
//		if (m_vsync_enabled)
//		{
//			// Lock to screen refresh rate.
//			m_swapChain->Present(1, 0);
//		}
//		else
//		{
//			// Present as fast as possible.
//			m_swapChain->Present(0, 0);
//		}
//
//		WaitForPreviousFrame();
//
//		// Clear pixel and vertex shader texture slots
//		// TODO: Likely unnecessary to clear all slots. Look at estimating max texture count
//		//ID3D11ShaderResourceView* nullViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
//		//m_deviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullViews);
//		//m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullViews);
//		//UnbindAllRenderTargets();
//		temp = "end";
//		return;
//	}
//
//	void DX12Impl::UnbindAllRenderTargets() const
//	{
//		//// Clear render target state
//		//ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
//		//m_deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullViews, nullptr);
//	}
//
//	void DX12Impl::UnbindShaderResourceView(uint32_t slot) const
//	{
//		/*EngineAssert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
//		ID3D11ShaderResourceView* nullView[1] = {};
//		m_deviceContext->VSSetShaderResources(slot, 1, nullView);
//		m_deviceContext->PSSetShaderResources(slot, 1, nullView);*/
//	}
//
//	void DX12Impl::GetVideoCardInfo(char* cardName, int& memory)
//	{
//		strcpy_s(cardName, 128, m_videoCardDescription.c_str());
//		memory = m_videoCardMemory;
//		return;
//	}
//
//	Utils::Maths::Vector2 DX12Impl::GetScreenSize() const
//	{
//		/*EngineAssert(m_backBufferRT != nullptr);
//
//		return Utils::Maths::Vector2(static_cast<float>(m_backBufferRT->GetWidth()), static_cast<float>(m_backBufferRT->GetHeight()));*/
//
//		return Utils::Maths::Vector2::Zero;
//	}
//
//	void DX12Impl::SetRenderTarget(RenderTargetBundle::Ptr bundle, Utils::Maths::Vector2 clipOffset /*= {}*/) const
//	{
//		//Utils::Maths::Vector2 viewSize;
//		//if (bundle == nullptr)
//		//{
//		//	m_deviceContext->OMSetRenderTargets(1, m_backBufferRT->GetRTV().GetAddressOf(), m_depthBuffer->GetDSV().Get());
//		//	viewSize = m_screenSize;
//		//}
//		//else
//		//{
//		//	ID3D11DepthStencilView* dsv = nullptr;
//		//	if (bundle->GetDepthBuffer() != nullptr) dsv = bundle->GetDepthBuffer()->GetDSV().Get();
//		//	m_deviceContext->OMSetRenderTargets(bundle->RenderTargetCount(), bundle->GetRenderTargetViews(), dsv);
//		//	viewSize = { float(bundle->GetWidth()), float(bundle->GetHeight()) };
//		//}
//
//		//// Set the correct viewport and scisssor rect depending on given data
//		//int left = int(clipOffset.x);
//		//int top = int(clipOffset.y);
//		//int right = int(viewSize.x - clipOffset.x);
//		//int bottom = int(viewSize.y - clipOffset.y);
//		//D3D11_RECT rect{ left, top, right, bottom };
//
//		//D3D11_VIEWPORT viewport;
//		//viewport.Width = viewSize.x;
//		//viewport.Height = viewSize.y;
//		//viewport.MinDepth = 0.0f;
//		//viewport.MaxDepth = 1.0f;
//		//viewport.TopLeftX = 0.0f;
//		//viewport.TopLeftY = 0.0f;
//
//		//m_deviceContext->RSSetViewports(1, &viewport);
//		//m_deviceContext->RSSetScissorRects(1, &rect);
//	}
//
//	Texture::Ptr DX12Impl::CopyTexture(Texture::Ptr textureToCopy) const
//	{
//		/*EngineAssert(textureToCopy != nullptr);
//		auto newTexture = Texture::CreateIdenticalTexture(textureToCopy, m_device.Get());
//
//		m_deviceContext->CopyResource(newTexture->GetTexture().Get(), textureToCopy->GetTexture().Get());
//
//		return newTexture;*/
//
//		return nullptr;
//	}
//
//	Texture::Ptr DX12Impl::CopyBackBuffer() const
//	{
//		//return CopyTexture(m_backBufferRT->GetTexture());
//		return nullptr;
//	}
//
//	void DX12Impl::BeginRenderEvent(const std::wstring& eventLabel)
//	{
//		/*if (m_userDefinedAnnotation != nullptr)
//		{
//			m_userDefinedAnnotation->BeginEvent(eventLabel.c_str());
//		}*/
//	}
//
//	void DX12Impl::EndRenderEvent()
//	{
//		/*if (m_userDefinedAnnotation != nullptr)
//		{
//			m_userDefinedAnnotation->EndEvent();
//		}*/
//	}
//
//	void DX12Impl::WaitForPreviousFrame()
//	{
//		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
//		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
//		// sample illustrates how to use fences for efficient resource usage and to
//		// maximize GPU utilization.
//
//		// Signal and increment the fence value.
//		const UINT64 fence = m_fenceValue;
//		Utils::DirectXHelpers::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
//		m_fenceValue++;
//
//		// Wait until the previous frame is finished.
//		if (m_fence->GetCompletedValue() < fence)
//		{
//			Utils::DirectXHelpers::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
//			WaitForSingleObject(m_fenceEvent, INFINITE);
//		}
//
//		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
//	}
//}