//
// EngineComponent.xaml.h
// Declaration of the EngineComponent class
//

#pragma once

#include "EngineComponent.g.h"
#include <Engine\Engine.h>
#include <concrt.h>

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
        
        void StartRendererThread();

    private:
        Engine::Ptr m_engine;

        // Input management
        Windows::Foundation::IAsyncAction^ m_inputLoopWorker;
        Windows::UI::Core::CoreIndependentInputSource^ m_coreInput;

        // Render thread
        Windows::Foundation::IAsyncAction^ m_renderThreadWorker;
        Concurrency::critical_section m_criticalSection;
    };
}
