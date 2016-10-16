//
// ScalarProperty.h
// Declaration of the ScalarProperty class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

namespace WinRT_App
{
    public ref class ScalarProperty sealed : public WUX::Controls::ContentControl
    {
    public:
        property double Value
        {
            double get() { return m_value; }
        private:
            void set(double value) { m_value = value; }
        }

        property double Minimum
        {
            double get() { return (double)GetValue(m_sliderMinimum); }
            void set(double value) { SetValue(m_sliderMinimum, value); }
        }

        property double Maximum
        {
            double get() { return (double)GetValue(m_sliderMaximum); }
            void set(double value) { SetValue(m_sliderMaximum, value); }
        }

    protected:
        virtual void OnApplyTemplate() override;

    internal:
        ScalarProperty(double initialValue, double min = 0.0, double max = 100.0);

    private:
        void OnSliderValueChanged(Platform::Object^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs^ args);
        void OnScalarTextBoxKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);

    private:
        // Scalar dependency properties
        static WUX::DependencyProperty^ m_sliderMinimum;
        static WUX::DependencyProperty^ m_sliderMaximum;

        // Scalar Controls
        WUX::Controls::Slider^ m_sliderElement;
        WUX::Controls::TextBox^ m_scalarTextBox;

        double m_value;
    };
}
