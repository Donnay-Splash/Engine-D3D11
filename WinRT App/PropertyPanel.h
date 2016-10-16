//
// PropertyPanel.h
// Declaration of the PropertyPanel class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

namespace WinRT_App
{
    public ref class PropertyPanel sealed : public WUX::Controls::ContentControl
    {
    public:
        PropertyPanel(Engine_WinRT::PropertyCX^ property);

    protected:
        virtual void OnApplyTemplate() override
        {
            WUX::Controls::ContentControl::OnApplyTemplate();

            // When the template has been applied we can bind to events.
            switch (m_property->Type)
            {
            case Engine_WinRT::PropertyType::Vector:
                break;

            case Engine_WinRT::PropertyType::Scalar:
                m_sliderElement = (WUX::Controls::Slider^)GetTemplateChild("Slider");
                if (m_sliderElement != nullptr)
                {
                    // Set the current value
                    m_sliderElement->Value = m_property->Scalar;

                    m_sliderElement->ValueChanged += 
                        ref new WUX::Controls::Primitives::RangeBaseValueChangedEventHandler(this, &PropertyPanel::OnSliderValueChanged);

                    m_textBox = (WUX::Controls::TextBox^)GetTemplateChild("TextBox");
                    if (m_textBox != nullptr)
                    {
                        m_textBox->KeyDown += ref new Windows::UI::Xaml::Input::KeyEventHandler(this, &PropertyPanel::OnTextBoxKeyDown);
                    }
                }
                break;

            case Engine_WinRT::PropertyType::Bool:
            default:
                throw ref new Platform::Exception(E_INVALIDARG, "Invalid property type received");
                break;
            }
        }

        property double SliderMinimum
        {
            double get() { return (double)GetValue(m_sliderMinimum); }
            void set(double value) { SetValue(m_sliderMinimum, value); }
        }

        property double SliderMaximum
        {
            double get() { return (double)GetValue(m_sliderMaximum); }
            void set(double value) { SetValue(m_sliderMaximum, value); }
        }

    private:
        // Scalar event handlers
        void OnSliderValueChanged(Platform::Object^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs^ args);
        void OnTextBoxKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);

    private:
        static WUX::DependencyProperty^ m_sliderMinimum;
        static WUX::DependencyProperty^ m_sliderMaximum;

        // Scalar Controls
        WUX::Controls::Slider^ m_sliderElement;
        WUX::Controls::TextBox^ m_textBox;

        // Vector Controls

        // Boolean Controls

        Engine_WinRT::PropertyCX^ m_property;
    };
}
