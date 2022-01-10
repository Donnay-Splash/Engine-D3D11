#include "pch.h"
#include "UWP_AppMain.h"
#include "Common\DirectXHelper.h"

using namespace UWP_App;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// The DirectX 12 Application template is documented at https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x409

// Loads and initializes application assets when the application is loaded.
UWP_AppMain::UWP_AppMain()
{
	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

// Creates and initializes the renderers.
void UWP_AppMain::CreateRenderers(IUnknown* window, uint32_t width, uint32_t height)
{
    // Create the application wrapper object.
    m_engine = std::make_unique<Engine::Engine>();

    // Initialize the application wrapper object.

    Engine::EngineCreateOptions createOptions;
    createOptions.RendererMode = Engine::EngineRendererMode::UWP;
	createOptions.CoreWindow = window;
    createOptions.ScreenWidth = width;
    createOptions.ScreenHeight = height;
    createOptions.FullScreen = false;
    createOptions.VSyncEnabled = true;
    bool result = m_engine->Initialize(createOptions);
	if (!result)
		throw;

}

// Updates the application state once per frame.
void UWP_AppMain::Update()
{
	// Update scene objects.
	// Potebtially update input here
	m_engine->SetFrameInput(m_currentInputState);
	m_currentInputState.WheelDelta = 0.0f;
	m_currentInputState.KeysPressed.clear();
	m_currentInputState.KeysReleased.clear();
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool UWP_AppMain::Render()
{
	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	if(m_engine)
		return m_engine->Frame();

	return false;
}

// Updates application state when the window's size changes (e.g. device orientation change)
void UWP_AppMain::OnWindowSizeChanged(uint32_t newWidth, uint32_t newHeight)
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_engine->ResizeBuffers(newWidth, newHeight);
}

// Notifies the app that it is being suspended.
void UWP_AppMain::OnSuspending()
{
	// TODO: Replace this with your app's suspending logic.

	// Process lifetime management may terminate suspended apps at any time, so it is
	// good practice to save any state that will allow the app to restart where it left off.

	// If your application uses video memory allocations that are easy to re-create,
	// consider releasing that memory to make it available to other applications.
}

// Notifes the app that it is no longer suspended.
void UWP_AppMain::OnResuming()
{
	// TODO: Replace this with your app's resuming logic.
}

// Notifies renderers that device resources need to be released.
void UWP_AppMain::OnDeviceRemoved()
{
	// TODO: Save any necessary application or renderer state and release the renderer
	// and its resources which are no longer valid.
	m_engine = nullptr;
}

void UWP_AppMain::OnPointerPressed(uint32_t mouseButtons)
{
	m_currentInputState.MouseButtonPressed |= mouseButtons;
}

void UWP_AppMain::OnPointerReleased(uint32_t mouseButtons)
{
	m_currentInputState.MouseButtonPressed = 0;
}

void UWP_AppMain::OnPointerMoved(float x, float y)
{
	m_currentInputState.PointerPosition.x = x;
	m_currentInputState.PointerPosition.y = y;
}

void UWP_AppMain::OnPointerWheelChanged(float wheelDelta)
{
	m_currentInputState.WheelDelta += wheelDelta;
}

void UWP_AppMain::OnKeyDown(uint32_t key)
{
    m_currentInputState.KeysPressed.insert(key);
}

void UWP_AppMain::OnKeyReleased(uint32_t key)
{
	m_currentInputState.KeysReleased.insert(key);
}