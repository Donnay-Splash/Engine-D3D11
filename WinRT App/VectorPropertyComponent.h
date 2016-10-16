//
// VectorPropertyComponent.h
// Declaration of the VectorPropertyComponent class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

namespace WinRT_App
{
    public delegate void VectorComponentValueChangedEventHandler(Platform::Object^ sender, double newValue);

    public ref class VectorPropertyComponent sealed : public WUX::Controls::ContentControl
    {
    public:
        VectorPropertyComponent();

        property double Value
        {
            double get() { return m_value; }
            void set(double value) 
            {
                m_value = value; 
                SetTextBoxValue();
                ValueChanged((Platform::Object^)this, value);
            }
        }

        property double Minimum
        {
            double get() { return m_minimum; }
            void set(double min) { m_minimum = min; };
        }

        property double Maximum
        {
            double get() { return m_maximum; }
            void set(double max) { m_maximum = max; }
        }

        event VectorComponentValueChangedEventHandler^ ValueChanged;

    protected:
        virtual void OnApplyTemplate() override;

    internal:

    private:
        Platform::String^ GetValueAsString();
        double GetValueFromTextBox(Platform::String^ text);
        void SetCursor(Windows::UI::Core::CoreCursorType type);
        void UpdateValueFromDrag(double delta);
        void SetTextBoxValue();

        // Event handlers
        void OnKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);
        void OnPointerEntered(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerPressed(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerMoved(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerReleased(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerCaptureLost(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);

    private:
        double m_value;
        double m_minimum;
        double m_maximum;
        
        const double m_defaultDragControlFactor = 0.05;
        const double m_drageRangeControlFactor = 0.005;

        WUX::Controls::TextBox^ m_textBox;

        // Mouse information
        bool m_pressed = false;
        bool m_captured = false;
        Windows::Foundation::Point m_prevPosition;
    };
}
