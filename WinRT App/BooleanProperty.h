//
// BooleanProperty.h
// Declaration of the BooleanProperty class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

namespace WinRT_App
{
    public delegate void BooleanPropertyChangedEventHandler(Platform::Object^ sender, bool newValue);

    public ref class BooleanProperty sealed : public WUX::Controls::ContentControl
    {
    public:
        property bool Value
        {
            bool get() { return m_value; }
            void set(bool value)
            {
                if (m_value != value)
                {
                    m_value = value;
                    ValueChanged((Platform::Object^)this, value);
                }
            }
        }

        event BooleanPropertyChangedEventHandler^ ValueChanged;

    internal:
        BooleanProperty(bool initialValue);

    protected:
        virtual void OnApplyTemplate() override;

    private:
        void OnChecked(Platform::Object^ sender, WUX::RoutedEventArgs^ args);
        void OnUnchecked(Platform::Object^ sender, WUX::RoutedEventArgs^ args);

    private:
        bool m_value;
    };
}
