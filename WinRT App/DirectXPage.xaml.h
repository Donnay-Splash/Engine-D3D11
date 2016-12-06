//
// DirectXPage.xaml.h
// Declaration of the DirectXPage class.
//

#pragma once

#include "DirectXPage.g.h"

#include "Common\DeviceResources.h"
#include "WinRT_AppMain.h"
#include "ExpandPanel.h"
#include <ppltasks.h>

namespace WinRT_App
{
    /// <summary>
    /// A page that hosts a DirectX SwapChainPanel.
    /// </summary>
    public ref class DirectXPage sealed
    {
    public:
        DirectXPage();
        virtual ~DirectXPage();
        void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
        void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

    private:
        // XAML low-level rendering event handler.
        void OnRendering(Platform::Object^ sender, Platform::Object^ args);

        // Window event handlers.
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

        // DisplayInformation event handlers.
        void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
        void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
        void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

        // Engine specific event handler
        void OnSceneElementAdded(Engine_WinRT::SceneElementCX^ sceneElement);

        // Window event handler
        void OnWindowActivated(Platform::Object^ sender, Windows::UI::Core::WindowActivatedEventArgs^ e);
        void OnTitleBarVisibilityChanged(Windows::ApplicationModel::Core::CoreApplicationViewTitleBar^ titleBar, Platform::Object^ args);
        void OnLayoutMetricsChanged(Windows::ApplicationModel::Core::CoreApplicationViewTitleBar^ titleBar, Platform::Object^ args);

        Concurrency::task<void> OpenFile();

    private:
        void TitleBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        std::map <uint32_t, ExpandPanel^> m_uiMap;
        void OpenFileButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    };
}

