//
// EngineComponent.xaml.h
// Declaration of the EngineComponent class
//

#pragma once

#include "EngineComponent.g.h"
#include <Engine\Engine.h>

namespace Engine_WinRT
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class EngineComponent sealed
    {
    public:
        EngineComponent();
        virtual ~EngineComponent();

    private:
        // SwapChainPanel events
        void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
        void OnSwapChainPanelSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);

        // Independent input handling functions.
        void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
        void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

        static void InitializeSwapChain(IUnknown* swapChain, void* userData);

    private:
        Engine::Ptr m_engine;

        // Input management
        Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
        Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;
    };
}
