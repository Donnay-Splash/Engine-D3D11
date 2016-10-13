//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include <experimental\resumable>
#include <ppltasks.h>
#include <pplawait.h>

using namespace WinRT_App;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage;
using namespace concurrency;

DirectXPage::DirectXPage()
{
    InitializeComponent();

    //engineComponent->SceneElementAdded += ref new Engine_WinRT::SceneElementAddedEventHandler(this, &DirectXPage::OnSceneElementAdded);

    //// Register event handlers for page lifecycle.
    //CoreWindow^ window = Window::Current->CoreWindow;

    //window->VisibilityChanged +=
    //    ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

    //DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

    //currentDisplayInformation->DpiChanged +=
    //    ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

    //currentDisplayInformation->OrientationChanged +=
    //    ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

    //DisplayInformation::DisplayContentsInvalidated +=
    //    ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);


    /*swapChainPanel->CompositionScaleChanged += 
        ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

    swapChainPanel->SizeChanged +=
        ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);*/

    // At this point we have access to the device. 
    // We can create the device-dependent resources.
    //m_deviceResources = std::make_shared<DX::DeviceResources>();
    //m_deviceResources->SetSwapChainPanel(swapChainPanel);

    // Register our SwapChainPanel to get independent input pointer events
    //auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
    //{
    //    // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
    //    m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
    //        Windows::UI::Core::CoreInputDeviceTypes::Mouse |
    //        Windows::UI::Core::CoreInputDeviceTypes::Touch |
    //        Windows::UI::Core::CoreInputDeviceTypes::Pen
    //        );

    //    // Register for pointer events, which will be raised on the background thread.
    //    m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
    //    m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
    //    m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

    //    // Begin processing input messages as they're delivered.
    //    m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    //});

    // Run task on a dedicated high priority background thread.
    /*m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

    m_main = std::unique_ptr<WinRT_AppMain>(new WinRT_AppMain(m_deviceResources));
    m_main->StartRenderLoop();*/
}

DirectXPage::~DirectXPage()
{
    // Stop rendering and processing events on destruction.
    //m_main->StopRenderLoop();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
    //critical_section::scoped_lock lock(m_main->GetCriticalSection());
    //m_deviceResources->Trim();

    // Stop rendering when the app is suspended.
    //m_main->StopRenderLoop();

    // Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
    // Put code to load app state here.

    // Start rendering when the app is resumed.
    //m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    //m_windowVisible = args->Visible;
    //if (m_windowVisible)
    //{
    //    //m_main->StartRenderLoop();
    //}
    //else
    //{
    //   //m_main->StopRenderLoop();
    //}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
    //critical_section::scoped_lock lock(m_main->GetCriticalSection());
    // Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
    // if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
    // you should always retrieve it using the GetDpi method.
    // See DeviceResources.cpp for more details.
    //m_deviceResources->SetDpi(sender->LogicalDpi);
    //m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
    //critical_section::scoped_lock lock(m_main->GetCriticalSection());
    //m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
    //m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
    //critical_section::scoped_lock lock(m_main->GetCriticalSection());
    //m_deviceResources->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    // Use the app bar if it is appropriate for your app. Design the app bar, 
    // then fill in event handlers (like this one).
    OpenFile();
}

Concurrency::task<void> DirectXPage::OpenFile()
{
    auto picker = ref new Pickers::FileOpenPicker();
    picker->FileTypeFilter->Append(L".mike");

    auto file = co_await picker->PickSingleFileAsync();
    if (file != nullptr)
    {
        engineComponent->LoadFile(file);
    }
    // Best Idea is to pass the data to EngineComponent. do this and then lock and pass to engine to load.
}

void DirectXPage::OnSceneElementAdded(Engine_WinRT::SceneElementCX^ sceneElement)
{
    // Now we have the scene element we need to build UI out of it.

}
