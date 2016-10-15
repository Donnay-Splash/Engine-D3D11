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
        void OnSliderValueChanged(Platform::Object^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs^ args);

    private:
        static WUX::DependencyProperty^ m_sliderMinimum;
        static WUX::DependencyProperty^ m_sliderMaximum;

        WUX::Controls::Slider^ m_sliderElement;

        Engine_WinRT::PropertyCX^ m_property;
    };
}
