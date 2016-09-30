//
// EngineComponent.xaml.h
// Declaration of the EngineComponent class
//

#pragma once

#include "EngineComponent.g.h"
#include <Engine.h>
#include <InputManager.h>
#include <concrt.h>
#include <ppltasks.h>

namespace Engine_WinRT
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class EngineComponent sealed
    {
    public:
        EngineComponent();
        virtual ~EngineComponent();

        void LoadFile(Windows::Storage::StorageFile^ fileToLoad);

    private:
        // SwapChainPanel events
        void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
        void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
        
        void OnUserControlLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        // Independent input handling functions.
        void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnKeyUp(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);
        void OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);

        static void InitializeSwapChain(IUnknown* swapChain, void* userData);
        
        void StartRendererThread();

        Concurrency::task<void> LoadFileInternal(Windows::Storage::StorageFile^ fileToLoad);

    private:
        Engine::Ptr m_engine;

        // Input management
        Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
        Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

        // Render thread
        Windows::Foundation::IAsyncAction^ m_renderThreadWorker;
        Concurrency::critical_section m_renderingMutex;
        Concurrency::critical_section m_inputMutex;

        Windows::Foundation::Size m_swapChainSize;

        InputState m_inputState;
    };
}
