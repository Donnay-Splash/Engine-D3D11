#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace UWP_App;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using Microsoft::WRL::ComPtr;

// The DirectX 12 Application template is documented at https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x409

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

    // Declare the pointer event handlers.
    window->PointerPressed +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerPressed);
	window->PointerEntered +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerEntered);
	window->PointerReleased +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerReleased);
	window->PointerExited +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerExited);
	window->PointerCaptureLost +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerCaptureLost);
	window->PointerMoved +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerMoved);
	window->PointerWheelChanged +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerWheelChanged);

	window->KeyDown += 
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyDown);
    window->KeyUp +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyUp);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<UWP_AppMain>(new UWP_AppMain());
	}
}

// This method is called after the window becomes active.
void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            m_main->Update();
			m_main->Render();
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();

	m_main->CreateRenderers(reinterpret_cast<IUnknown*>(CoreWindow::GetForCurrentThread()),
		static_cast<uint32_t>(CoreWindow::GetForCurrentThread()->Bounds.Width),
		static_cast<uint32_t>(CoreWindow::GetForCurrentThread()->Bounds.Height));
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		m_main->OnSuspending();
		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	m_main->OnResuming();
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_main->OnWindowSizeChanged(static_cast<uint32_t>(sender->Bounds.Width), static_cast<uint32_t>(sender->Bounds.Height));
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}

// Input handling
void App::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	args->Handled = true;
	
	sender->SetPointerCapture();

	uint32_t buttonsMask = 0;
	if (args->CurrentPoint->Properties->IsLeftButtonPressed)
	{
		buttonsMask |= Engine::MouseButtons::Left;
	}

    if (args->CurrentPoint->Properties->IsMiddleButtonPressed)
    {
        buttonsMask |= Engine::MouseButtons::Middle;
    }

	if (args->CurrentPoint->Properties->IsRightButtonPressed)
	{
		buttonsMask |= Engine::MouseButtons::Right;
	}


	m_main->OnPointerPressed(buttonsMask);
}

void App::OnPointerEntered(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{

}

void App::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    args->Handled = true;

    sender->ReleasePointerCapture();

    uint32_t buttonsMask = 0;
    m_main->OnPointerReleased(buttonsMask);
}

void App::OnPointerExited(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{

}

void App::OnPointerCaptureLost(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{

}

void App::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
	args->Handled = true;

	m_main->OnPointerMoved(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
}

void App::OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    args->Handled = true;

	float wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / 5.0f;

    m_main->OnPointerWheelChanged(wheelDelta);
}

void App::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
{
	args->Handled = true;

	m_main->OnKeyDown((uint32_t)args->VirtualKey);
}

void App::OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
{
    args->Handled = true;

    m_main->OnKeyReleased((uint32_t)args->VirtualKey);
}

