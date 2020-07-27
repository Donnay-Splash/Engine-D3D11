////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "d3dclass.h"
#include "DX12Impl.h"
#include <locale>
#include <codecvt>
#include <iomanip>

namespace Engine
{
	static D3DClass* s_Instance = nullptr;
	void D3DClass::Initialize(EngineCreateOptions createOptions)
	{
		// TODO: Add platform to create options so that we can dynamically switch
		EngineAssert(s_Instance == nullptr);
		s_Instance = new D3DClass();
		s_Instance->Initialize_Internal(createOptions);
	}

	D3DClass* D3DClass::Instance()
	{
		EngineAssert(s_Instance != nullptr);
		return s_Instance;
	}

	D3DClass::D3DClass()
	{
		m_swapChain = nullptr;
		m_device = nullptr;
		//m_deviceContext = nullptr;
	}


	D3DClass::D3DClass(const D3DClass& other)
	{
	}


	D3DClass::~D3DClass()
	{
		//m_deviceContext->ClearState();
		m_debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_FLAGS::D3D12_RLDO_DETAIL);
	}


	void D3DClass::Initialize_Internal(EngineCreateOptions createOptions)
	{
		// Store the vsync setting.
		m_createOptions = createOptions;
		m_vsync_enabled = m_createOptions.VSyncEnabled;

		EngineAssert(m_createOptions.ScreenWidth > 0);
		EngineAssert(m_createOptions.ScreenHeight > 0);
		m_screenSize = { static_cast<float>(m_createOptions.ScreenWidth), static_cast<float>(m_createOptions.ScreenHeight) };

		UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
		Utils::DirectXHelpers::ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();

		Utils::DirectXHelpers::ThrowIfFailed(debugController->QueryInterface(IID_PPV_ARGS(&debugController1)));
		debugController1->SetEnableGPUBasedValidation(TRUE);

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		// Create a DirectX graphics interface factory.
		Utils::DirectXHelpers::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));

		// Retrieve information from the adapter.
		GetAdapterInformation();

		// Create the device and swap chain
		CreateDeviceResourcesAndSwapChain();
	}

	void D3DClass::GetAdapterInformation()
	{
		Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;
		DXGI_ADAPTER_DESC adapterDesc;

		// Use the factory to create an adapter for the primary graphics interface (video card).
		Utils::DirectXHelpers::ThrowIfFailed(m_factory->EnumAdapters(0, m_adapter.GetAddressOf()));

		// Get the adapter (video card) description.
		Utils::DirectXHelpers::ThrowIfFailed(m_adapter->GetDesc(&adapterDesc));

		// Store the dedicated video card memory in megabytes.
		m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

		// Convert the name of the video card from wide to standard character length.
		std::wstring wideAdapterDescription(adapterDesc.Description);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		m_videoCardDescription = converter.to_bytes(wideAdapterDescription);
	}

	void D3DClass::CreateDeviceResourcesAndSwapChain()
	{
		// Set the feature level to DirectX 11.
		D3D_FEATURE_LEVEL featureLevelsSupported[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};

		Utils::DirectXHelpers::ThrowIfFailed(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
		// Query the debug device from the device
		Utils::DirectXHelpers::ThrowIfFailed(m_device.As(&m_debugDevice));

		// Create the command queue, allocator and list
		CreateCommandList();

		// Now create all of our global descriptor heaps.
		CreateDescriptorHeaps();

		switch (m_createOptions.RendererMode)
		{
		case EngineRendererMode::Win32:
			CreateSwapChain_HWND(m_createOptions.ScreenWidth, m_createOptions.ScreenHeight);
			break;
		case EngineRendererMode::XAML:
			CreateSwapChain_XAML(m_createOptions.ScreenWidth, m_createOptions.ScreenHeight);
			break;
		default:
			// Incorrect enum value passed in
			EngineAssert(false);
			break;
		}

		// Now call swapchain created callback.
		if (m_createOptions.SwapChainCreatedCallback != nullptr)
		{
			m_createOptions.SwapChainCreatedCallback(m_swapChain.Get(), m_createOptions.UserData);
		}
	}

	void D3DClass::CreateDescriptorHeaps()
	{
		// Size's are likely too small or big for these. Need to find more suitable values
		m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = std::make_unique<DescriptorAllocator>(20, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = std::make_unique<DescriptorAllocator>(20, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = std::make_unique<DescriptorAllocator>(20, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	void D3DClass::FreeDescriptor(DescriptorPair* descriptor)
	{
		bool freedDescriptor = false;
		for (auto& heap : m_DescriptorHeaps)
		{
			if (heap.second->BelongsToHeap(descriptor))
			{
				heap.second->FreeDescriptor(descriptor);
				freedDescriptor = true;
				break;
			}
		}

		EngineAssert(freedDescriptor);
	}

	void D3DClass::CreateCommandList()
	{
		// Create the command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		// Create all of our command queues
		m_commandQueues[D3D12_COMMAND_LIST_TYPE_DIRECT] = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_commandQueues[D3D12_COMMAND_LIST_TYPE_COMPUTE] = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		m_commandQueues[D3D12_COMMAND_LIST_TYPE_COPY] = std::make_unique<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);

		// Create synchronisation objects
		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create event handle for frame synchronization
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			Utils::DirectXHelpers::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

	void D3DClass::CreateSwapChain_HWND(uint32_t screenWidth, uint32_t screenHeight)
	{
		if (m_swapChain == nullptr)
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
			SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

			// Set to a single back buffer.
			swapChainDesc.BufferCount = kBufferCount;
			swapChainDesc.Width = screenWidth;
			swapChainDesc.Height = screenHeight;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			// Set the usage of the back buffer.
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

			// Turn multisampling off.
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;

			// Discard the back buffer contents after presenting.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			// Don't set the advanced flags.
			swapChainDesc.Flags = 0;

			// Create the swap chain
			Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
			auto hr = m_factory->CreateSwapChainForHwnd(GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue(), m_createOptions.HWND, &swapChainDesc, NULL, NULL, tempSwapChain.GetAddressOf());
			Utils::DirectXHelpers::ThrowIfFailed(hr);

			// Convert to SwapChain3
			Utils::DirectXHelpers::ThrowIfFailed(tempSwapChain.As(&m_swapChain));
		}
		else
		{
			// Resize swap chain
			// TODO: Need to remember to release back buffers.
			m_swapChain->ResizeBuffers(kBufferCount, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		}

		CreateBackBufferResources(screenWidth, screenHeight);
	}

	void D3DClass::CreateSwapChain_XAML(uint32_t screenWidth, uint32_t screenHeight)
	{
		if (m_swapChain == nullptr)
		{
			//DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
			//SecureZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

			//// Set to a single back buffer.
			//swapChainDesc.BufferCount = kBufferCount;
			//swapChainDesc.Width = screenWidth;
			//swapChainDesc.Height = screenHeight;
			//swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			//// Set the usage of the back buffer.
			//swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

			//// Turn multisampling off.
			//swapChainDesc.SampleDesc.Count = 1;
			//swapChainDesc.SampleDesc.Quality = 0;

			//// Discard the back buffer contents after presenting.
			//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			//// Don't set the advanced flags.
			//swapChainDesc.Flags = 0;

			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

			swapChainDesc.Width = lround(screenWidth);		// Match the size of the window.
			swapChainDesc.Height = lround(screenHeight);
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// This is the most common swap chain format.
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Windows Store apps must use _FLIP_ SwapEffects.
			swapChainDesc.Flags = 0;
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			// Create the swap chain
			Microsoft::WRL::ComPtr<IDXGISwapChain1> tempSwapChain;
			Utils::DirectXHelpers::ThrowIfFailed(m_factory->CreateSwapChainForComposition(GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue(), &swapChainDesc, nullptr, tempSwapChain.GetAddressOf()));

			// Convert to SwapChain3
			Utils::DirectXHelpers::ThrowIfFailed(tempSwapChain.As(&m_swapChain));
		}
		else
		{
			// Resize swap chain
			// TODO: Need to remember to release back buffers.
			auto hr = (m_swapChain->ResizeBuffers(kBufferCount, screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
			if (FAILED(hr))
			{
				hr = m_device->GetDeviceRemovedReason();
				Utils::DirectXHelpers::ThrowIfFailed(hr);
			}
		}

		CreateBackBufferResources(screenWidth, screenHeight);
	}

	void D3DClass::CreateBackBufferResources(uint32_t screenWidth, uint32_t screenHeight)
	{
		// Create an RTV for each frame
		for (int n = 0; n < kBufferCount; ++n)
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
			Utils::DirectXHelpers::ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffer)));
			std::wstring name = L"BackBuffer:" + std::to_wstring(n);
			backBuffer->SetName(name.c_str());
			m_renderTargets[n] = std::make_unique<RenderTarget>(backBuffer.Get(), D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		// Eventually come back to this
		m_screenSize = { float(screenWidth), float(screenHeight) };

		//m_depthBuffer = std::make_shared<DepthBuffer>(backBufferWidth, backBufferHeight, 1, 0, m_device.Get());

		// Setup the viewport for rendering.
		m_viewport.Width = m_screenSize.x;
		m_viewport.Height = m_screenSize.y;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_viewport.TopLeftX = 0.0f;
		m_viewport.TopLeftY = 0.0f;

		// Setup scissor for rendering
		m_scissor = { 0 };
		m_scissor.right = screenWidth;
		m_scissor.bottom = screenHeight;
	}

	void D3DClass::ClearResources()
	{
		for (int n = 0; n < kBufferCount; ++n)
		{
			m_renderTargets[n] = nullptr;
		}

		/*m_backBufferRT = nullptr;
		m_depthBuffer = nullptr;

		m_deviceContext->Flush();*/
	}

	void D3DClass::Shutdown()
	{
		WaitForPreviousFrame();

		// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
		if (m_swapChain)
		{
			m_swapChain->SetFullscreenState(false, NULL);
		}

		//m_deviceContext = nullptr;
		m_device = nullptr;
		m_swapChain = nullptr;
		CloseHandle(m_fenceEvent);
	}

	void D3DClass::ResizeBuffers(uint32_t newWidth, uint32_t newHeight)
	{
		// TODO: Come up with way to stop unnecessary resize
		//if (newWidth != m_backBufferRT->GetWidth() || newHeight != m_backBufferRT->GetHeight())
		{
			ClearResources();
			// Resize buffers
			switch (m_createOptions.RendererMode)
			{
			case EngineRendererMode::Win32:
				CreateSwapChain_HWND(newWidth, newHeight);
				break;
			case EngineRendererMode::XAML:
				CreateSwapChain_XAML(newWidth, newHeight);
				break;
			default:
				// Incorrect enum value passed in
				EngineAssert(false);
				break;
			}

		}
		// otherwise continue as normal
	}


	void D3DClass::BeginScene(float red, float green, float blue, float alpha)
	{
		m_commandList = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandList();

		// Transition the back buffer back to render target
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex]->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		float color[4];
		// Setup the color to clear the buffer to.
		color[0] = red;
		color[1] = green;
		color[2] = blue;
		color[3] = alpha;

		// Clear the back buffer.
		m_commandList->ClearRenderTargetView(m_renderTargets[m_frameIndex]->GetCPUHandle(), color, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &m_renderTargets[m_frameIndex]->GetCPUHandle(), false, nullptr);
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_scissor);

		// Clear the depth buffer.
		//m_commandList->ClearDepthStencilView(m_depthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}


	void D3DClass::EndScene()
	{
		// Transition the back-buffer to present 
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex]->GetTexture()->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// Execute our command list
		GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->ExecuteCommandList(m_commandList);

		// null command list after execution, we'll grab another at the start of the next frame.
		m_commandList = nullptr;

		// Present the back buffer to the screen since rendering is complete.
		if (m_vsync_enabled)
		{
			// Lock to screen refresh rate.
			m_swapChain->Present(1, 0);
		}
		else
		{
			// Present as fast as possible.
			m_swapChain->Present(0, 0);
		}

		WaitForPreviousFrame();

		// Clear pixel and vertex shader texture slots
		// TODO: Likely unnecessary to clear all slots. Look at estimating max texture count
		//ID3D11ShaderResourceView* nullViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
		//m_deviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullViews);
		//m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullViews);
		//UnbindAllRenderTargets();
		return;
	}

	void D3DClass::UnbindAllRenderTargets() const
	{
		//// Clear render target state
		//ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
		//m_deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullViews, nullptr);
	}

	void D3DClass::UnbindShaderResourceView(uint32_t slot) const
	{
		/*EngineAssert(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		ID3D11ShaderResourceView* nullView[1] = {};
		m_deviceContext->VSSetShaderResources(slot, 1, nullView);
		m_deviceContext->PSSetShaderResources(slot, 1, nullView);*/
	}

	void D3DClass::SetShaderVisibleDescriptorHeaps()
	{
		EngineAssert(m_commandList);
		// Upload descriptor heaps.
		// TODO: Move to descriptor heap manager/allocator class.
		ID3D12DescriptorHeap* heaps[] = { m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetHeap() };
		m_commandList->SetDescriptorHeaps(1, heaps);
	}

	void D3DClass::UploadSubresourceData(uint64_t bufferSize, D3D12_SUBRESOURCE_DATA* subresourceData, ID3D12Resource* destination, D3D12_RESOURCE_STATES intendedState)
	{
		// Create our upload heap.
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediate;
		Utils::DirectXHelpers::ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(intermediate.GetAddressOf())));

		// Get a command list for copying resources
		auto copyCommandList = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY)->GetCommandList();
		UpdateSubresources(copyCommandList.Get(), destination, intermediate.Get(), 0, 0, 1, subresourceData);

		// After updating subresource transition to intended usage state
		// TODO: Can't perform this action on copy queue needs to be performed on direct queue. Not sure when maybe before drawing?
		//copyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(destination, D3D12_RESOURCE_STATE_COPY_DEST, intendedState));

		GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY)->ExecuteCommandList(copyCommandList);
		// Need to keep this alive until commands have finished.
		// Ideally we should create an automatic keep alive queue when resources are released.
		// Temporarily wait for idle instead to ensure commands are finished.
		GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY)->WaitForIdle();
		copyCommandList = nullptr;
	}

	void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
	{
		strcpy_s(cardName, 128, m_videoCardDescription.c_str());
		memory = m_videoCardMemory;
		return;
	}

	Utils::Maths::Vector2 D3DClass::GetScreenSize() const
	{
		/*EngineAssert(m_backBufferRT != nullptr);

		return Utils::Maths::Vector2(static_cast<float>(m_backBufferRT->GetWidth()), static_cast<float>(m_backBufferRT->GetHeight()));*/

		return Utils::Maths::Vector2::Zero;
	}

	void D3DClass::SetRenderTarget(RenderTargetBundle::Ptr bundle, Utils::Maths::Vector2 clipOffset /*= {}*/) const
	{
		//Utils::Maths::Vector2 viewSize;
		//if (bundle == nullptr)
		//{
		//	m_deviceContext->OMSetRenderTargets(1, m_backBufferRT->GetRTV().GetAddressOf(), m_depthBuffer->GetDSV().Get());
		//	viewSize = m_screenSize;
		//}
		//else
		//{
		//	ID3D11DepthStencilView* dsv = nullptr;
		//	if (bundle->GetDepthBuffer() != nullptr) dsv = bundle->GetDepthBuffer()->GetDSV().Get();
		//	m_deviceContext->OMSetRenderTargets(bundle->RenderTargetCount(), bundle->GetRenderTargetViews(), dsv);
		//	viewSize = { float(bundle->GetWidth()), float(bundle->GetHeight()) };
		//}

		//// Set the correct viewport and scisssor rect depending on given data
		//int left = int(clipOffset.x);
		//int top = int(clipOffset.y);
		//int right = int(viewSize.x - clipOffset.x);
		//int bottom = int(viewSize.y - clipOffset.y);
		//D3D11_RECT rect{ left, top, right, bottom };

		//D3D11_VIEWPORT viewport;
		//viewport.Width = viewSize.x;
		//viewport.Height = viewSize.y;
		//viewport.MinDepth = 0.0f;
		//viewport.MaxDepth = 1.0f;
		//viewport.TopLeftX = 0.0f;
		//viewport.TopLeftY = 0.0f;

		//m_deviceContext->RSSetViewports(1, &viewport);
		//m_deviceContext->RSSetScissorRects(1, &rect);
	}

	Texture::Ptr D3DClass::CopyTexture(Texture::Ptr textureToCopy) const
	{
		/*EngineAssert(textureToCopy != nullptr);
		auto newTexture = Texture::CreateIdenticalTexture(textureToCopy, m_device.Get());

		m_deviceContext->CopyResource(newTexture->GetTexture().Get(), textureToCopy->GetTexture().Get());

		return newTexture;*/

		return nullptr;
	}

	Texture::Ptr D3DClass::CopyBackBuffer() const
	{
		//return CopyTexture(m_backBufferRT->GetTexture());
		return nullptr;
	}

	void D3DClass::BeginRenderEvent(const std::wstring& eventLabel)
	{
		/*if (m_userDefinedAnnotation != nullptr)
		{
			m_userDefinedAnnotation->BeginEvent(eventLabel.c_str());
		}*/
	}

	void D3DClass::EndRenderEvent()
	{
		/*if (m_userDefinedAnnotation != nullptr)
		{
			m_userDefinedAnnotation->EndEvent();
		}*/
	}

	// TODO: REplace this with correct command queue behaviour
	void D3DClass::WaitForPreviousFrame()
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
		// sample illustrates how to use fences for efficient resource usage and to
		// maximize GPU utilization.

		const UINT64 fence = m_fenceValue;
		Utils::DirectXHelpers::ThrowIfFailed(GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue()->Signal(m_fence.Get(), fence));
		m_fenceValue++;

		// Wait until the previous frame is finished.
		if (m_fence->GetCompletedValue() < fence)
		{
			Utils::DirectXHelpers::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}
}