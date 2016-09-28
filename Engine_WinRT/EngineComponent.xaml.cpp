//
// EngineComponent.xaml.cpp
// Implementation of the EngineComponent class
//

#include "pch.h"
#include "EngineComponent.xaml.h"
#include <d3d11_3.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <Utils\EngineHelpers.h>

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
    createOptions.ScreenWidth = static_cast<int>(std::max(logicalWidth, 1.0));
    createOptions.ScreenHeight = static_cast<int>(std::max(logicalHeight, 1.0));
    m_engine->Initialize(createOptions);

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
            Windows::UI::Core::CoreInputDeviceTypes::Pen);

        // Register for pointer events, which will be raised on the background thread.
        m_coreInput->PointerPressed += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &EngineComponent::OnPointerPressed);
        m_coreInput->PointerMoved += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &EngineComponent::OnPointerMoved);
        m_coreInput->PointerReleased += ref new TypedEventHandler<Platform::Object^, PointerEventArgs^>(this, &EngineComponent::OnPointerReleased);

        // Begin processing input messages as they're delivered.
        m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    });

    // Run task on a dedicated high priority background thread.
    m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

    StartRendererThread();
}

EngineComponent::~EngineComponent()
{
    // Stop processing events on destruction
    m_coreInput->Dispatcher->StopProcessEvents();
}

void Engine_WinRT::EngineComponent::StartRendererThread()
{
    // If the animation render loop is already running then do not start another thread.
    if (m_renderThreadWorker != nullptr && m_renderThreadWorker->Status == AsyncStatus::Started)
    {
        return;
    }

    // Create a task that will be run on a background thread.
    WeakReference weakRef(this);
    auto workItemHandler = ref new WorkItemHandler([weakRef](IAsyncAction ^ action)
    {
        // Calculate the updated frame and render once per vertical blanking interval.
        if (auto engineComponent = weakRef.Resolve<EngineComponent>())
        {
            while (action->Status == AsyncStatus::Started)
            {
                {
                    Concurrency::critical_section::scoped_lock lock(engineComponent->m_inputMutex);
                    engineComponent->m_engine->SetFrameInput(engineComponent->m_inputState);
                    engineComponent->m_inputState.KeysPressed.clear();
                    engineComponent->m_inputState.KeysReleased.clear();
                }
                {
                    Concurrency::critical_section::scoped_lock lock(engineComponent->m_renderingMutex);
                    engineComponent->m_engine->Frame();
                }
            }
        }
    });

    // Run task on a dedicated high priority background thread.
    m_renderThreadWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
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
    Concurrency::critical_section::scoped_lock lock(m_renderingMutex);
    m_swapChainSize.Height = e->NewSize.Height;
    m_swapChainSize.Width = e->NewSize.Width;
    m_engine->ResizeBuffers(static_cast<uint32_t>(m_swapChainSize.Width), static_cast<uint32_t>(m_swapChainSize.Height));
}



void Engine_WinRT::EngineComponent::InitializeSwapChain(IUnknown * swapChain, void * userData)
{
    IDXGISwapChain* swapChainPtr;
    swapChain->QueryInterface(IID_PPV_ARGS(&swapChainPtr));
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
        Utils::DirectXHelpers::ThrowIfFailed(panelNative->SetSwapChain(swapChainPtr));
    }, CallbackContext::Any));
}


void Engine_WinRT::EngineComponent::OnUserControlLoaded(Object^ sender, RoutedEventArgs^ e)
{
    CoreWindow^ window = Window::Current->CoreWindow;
    window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &EngineComponent::OnKeyUp);
    window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &EngineComponent::OnKeyDown);
}

void Engine_WinRT::EngineComponent::OnPointerPressed(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ e)
{
    auto pointer = e->CurrentPoint;
    auto pointerProperties = pointer->Properties;
    Concurrency::critical_section::scoped_lock lock(m_inputMutex);
    if(pointerProperties->IsLeftButtonPressed)
    {
        m_inputState.MouseButtonPressed |= MouseButtons::Left;
    }
    if (pointerProperties->IsMiddleButtonPressed)
    {
        m_inputState.MouseButtonPressed |= MouseButtons::Middle;
    }
    if (pointerProperties->IsRightButtonPressed)
    {
        m_inputState.MouseButtonPressed |= MouseButtons::Right;
    }

    e->Handled = true;
}

void Engine_WinRT::EngineComponent::OnPointerMoved(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ e)
{
    auto pointer = e->CurrentPoint;
    auto position = pointer->Position;
    Concurrency::critical_section::scoped_lock lock(m_inputMutex);
    m_inputState.PointerPosition = {position.X / m_swapChainSize.Width, position.Y / m_swapChainSize.Height};
    e->Handled = true;
}

void Engine_WinRT::EngineComponent::OnPointerReleased(Platform::Object ^ sender, Windows::UI::Core::PointerEventArgs ^ e)
{
    auto pointer = e->CurrentPoint;
    auto pointerProperties = pointer->Properties;
    Concurrency::critical_section::scoped_lock lock(m_inputMutex);
    m_inputState.MouseButtonPressed = 0;

    e->Handled = true;
}

void Engine_WinRT::EngineComponent::OnKeyUp(CoreWindow ^sender, KeyEventArgs ^args)
{
    auto virtualKey = args->VirtualKey;
    uint32_t keyCode = static_cast<uint32_t>(virtualKey);
    Concurrency::critical_section::scoped_lock lock(m_inputMutex);
    m_inputState.KeysReleased.insert(keyCode);
    args->Handled = true;
}

void Engine_WinRT::EngineComponent::OnKeyDown(CoreWindow ^sender, KeyEventArgs ^args)
{
    auto virtualKey = args->VirtualKey;
    uint32_t keyCode = static_cast<uint32_t>(virtualKey);
    Concurrency::critical_section::scoped_lock lock(m_inputMutex);
    static int count = 0;
    count++;
    if (count > 1)
    {
        auto keyStatus = args->KeyStatus;
        auto tempThinger = 5;
        tempThinger += count;
        count = 0;
    }
    m_inputState.KeysPressed.insert(keyCode);
    args->Handled = true;
}