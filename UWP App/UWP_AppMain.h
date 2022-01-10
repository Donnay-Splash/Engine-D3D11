#pragma once
#include <Engine.h>

// Renders Direct3D content on the screen.
namespace UWP_App
{
	class UWP_AppMain
	{
	public:
		UWP_AppMain();
		void CreateRenderers(IUnknown* window, uint32_t width, uint32_t height);
		void Update();
		bool Render();

		void OnWindowSizeChanged(uint32_t newWidth, uint32_t newHeight);
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

		void OnPointerPressed(uint32_t mouseButtons);
		void OnPointerReleased(uint32_t mouseButtons);
		void OnPointerMoved(float x, float y);
		void OnPointerWheelChanged(float wheelDelta);

		void OnKeyDown(uint32_t key);
		void OnKeyReleased(uint32_t key);
	private:
        std::unique_ptr<Engine::Engine> m_engine;

        Engine::InputState m_currentInputState;
	};
}