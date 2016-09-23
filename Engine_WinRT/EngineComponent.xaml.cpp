//
// EngineComponent.xaml.cpp
// Implementation of the EngineComponent class
//

#include "pch.h"
#include "EngineComponent.xaml.h"
#include <d3d11_3.h>
#include <windows.ui.xaml.media.dxinterop.h>

using namespace Engine_WinRT;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::System::Threading;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

EngineComponent::EngineComponent()
{
    InitializeComponent();

    // Now initialise Engine.
    // Need to find a way to specify that we want to render with XAML flip mode swap chain and receive the swap chain from the engine.
    double logicalWidth = swapChainPanel->ActualWidth;
    double logicalHeight = swapChainPanel->ActualHeight;

    m_engine = std::make_shared<Engine>();
    EngineCreateOptions createOptions;
    createOptions.FullScreen = false;
    createOptions.RendererMode = EngineRendererMode::XAML;
    createOptions.UserData = (void*)this;
    createOptions.Callback = &EngineComponent::InitializeSwapChain;
    createOptions.ScreenWidth = static_cast<int>(logicalWidth);
    createOptions.ScreenHeight = static_cast<int>(logicalHeight);
    m_engine->Initialize(createOptions);
    //m_engine->Initialize()

    // Register to SwapChainPanel events
    swapChainPanel->CompositionScaleChanged += ref new TypedEventHandler<SwapChainPanel^, Platform::Object ^>(this, &EngineComponent::OnCompositionScaleChanged);

    swapChainPanel->SizeChanged += ref new SizeChangedEventHandler(this, &EngineComponent::OnSwapChainPanelSizeChanged);

    // Register our SwapChainPanel to get independent input pointer events
    auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
    {
        // Note: an ID3D11SwapChain must be set on the SwapChainPanel and presented to atleast once to begin
        // receiving input.
        // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
        m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
            Windows::UI::Core::CoreInputDeviceTypes::Mouse |
            Windows::UI::Core::CoreInputDeviceTypes::Touch |
            Windows::UI::Core::CoreInputDeviceTypes::Pen
            );

        // Register for pointer events, which will be raised on the background thread.
        m_coreInput->PointerPressed += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &EngineComponent::OnPointerPressed);
        m_coreInput->PointerMoved += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &EngineComponent::OnPointerMoved);
        m_coreInput->PointerReleased += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &EngineComponent::OnPointerReleased);

        // Begin processing input messages as they're delivered.
        m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    });

    // Run task on a dedicated high priority background thread.
    m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

EngineComponent::~EngineComponent()
{
    // Stop processing events on destruction
    m_coreInput->Dispatcher->StopProcessEvents();
}

void Engine_WinRT::EngineComponent::OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^ sender, Object ^ args)
{
    //throw ref new Platform::NotImplementedException();
    // Update composition scale and resize SwapChain resources.
}

void Engine_WinRT::EngineComponent::OnSwapChainPanelSizeChanged(Platform::Object ^ sender, Windows::UI::Xaml::SizeChangedEventArgs ^ e)
{
    //throw ref new Platform::NotImplementedException();
    // Update scale and resize SwapChain resources.
}

void Engine_WinRT::EngineComponent::OnPointerPressed(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ e)
{
    throw ref new Platform::NotImplementedException();
}

void Engine_WinRT::EngineComponent::OnPointerMoved(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ e)
{
    throw ref new Platform::NotImplementedException();
}

void Engine_WinRT::EngineComponent::OnPointerReleased(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ e)
{
    throw ref new Platform::NotImplementedException();
}

void Engine_WinRT::EngineComponent::InitializeSwapChain(IUnknown * swapChain, void * userData)
{
    IDXGISwapChain* swapChainPtr;
    swapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
    auto engineComponent = reinterpret_cast<EngineComponent^>(userData);

    // Associate swap chain with SwapChainPanel
    // UI changes will need to be dispatched back to the UI thread
    engineComponent->swapChainPanel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]()
    {
        // Get backing native interface for SwapChainPanel
        Microsoft::WRL::ComPtr<ISwapChainPanelNative> panelNative;
        Utils::DirectXHelpers::ThrowIfFailed(
            reinterpret_cast<IUnknown*>(engineComponent->swapChainPanel)->QueryInterface(IID_PPV_ARGS(&panelNative))
        );

        Utils::DirectXHelpers::ThrowIfFailed(
            panelNative->SetSwapChain(swapChainPtr)
        );
    }, CallbackContext::Any));
}
