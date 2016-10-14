//
// CustomTitleBar.xaml.h
// Declaration of the CustomTitleBar class
//

#pragma once
#pragma warning(push)
#pragma warning(disable: 4691)

#include "CustomTitleBar.g.h"
using namespace Windows::UI::Xaml;

namespace WinRT_App
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class CustomTitleBar sealed : Microsoft::Xaml::Interactivity::IBehavior
    {
    public:
        CustomTitleBar();
        property Windows::UI::Xaml::DependencyObject^ AssociatedObject
        {
            virtual Windows::UI::Xaml::DependencyObject^ get() { return this; };
        private:
            //void set(DependencyObject^ object);
        }

        virtual void Attach(DependencyObject^ associatedObject)
        {
            auto newTitleBar = dynamic_cast<UIElement^>(associatedObject);
            if (newTitleBar == nullptr)
            {
                throw ref new Platform::Exception(E_INVALIDARG, "TitleBarBehaviour can only be attached to a UIElement");
            }

            Window::Current->SetTitleBar(newTitleBar);
        }

        virtual void Detach() {}

        property bool IsChromeless
        {
            bool get() { return (bool)GetValue(IsChromelessProperty); }
            void set(bool value) { SetValue(IsChromelessProperty, value); }
        }

    private:
        static void OnIsChromelessChanged(DependencyObject^ d, DependencyPropertyChangedEventArgs^ e)
        {
            Windows::ApplicationModel::Core::CoreApplication::GetCurrentView()->TitleBar->ExtendViewIntoTitleBar = (bool)e->NewValue;
        }

    private:
        static DependencyProperty^ IsChromelessProperty;
    };
}
#pragma warning(pop)
