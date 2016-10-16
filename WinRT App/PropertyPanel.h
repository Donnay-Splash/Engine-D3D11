//
// PropertyPanel.h
// Declaration of the PropertyPanel class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;
namespace WFN = Windows::Foundation::Numerics;

namespace WinRT_App
{
    // TODO: Probably want to look at changing Vector to 3 components
    // I cannot think of a time when we would need 4. We are not using
    // Quaternions directly and a Color picker would be used for colors.
    // TODO: Split this into different components
    // Especially split vector into a draggable thing + text box
    // and then just add 4 of those to property. Will make this far cleaner
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
                ApplyVectorTemplate();
                break;

            case Engine_WinRT::PropertyType::Scalar:
                ApplyScalarTemplate();
                break;

            case Engine_WinRT::PropertyType::Bool:
            default:
                throw ref new Platform::Exception(E_INVALIDARG, "Invalid property type received");
                break;
            }
        }

        // Scalar properties
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
        // Scalar functions 
        void ApplyScalarTemplate();
        // Event handlers
        void OnSliderValueChanged(Platform::Object^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs^ args);
        void OnScalarTextBoxKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);

        // Vector functions
        void ApplyVectorTemplate();
        void GetValuesForComponent(Platform::String^ componentName, const WFN::float4& vectorValue, double& current, double& min, double& max);
        void SetValueForComponent(Platform::String^ componentName, WFN::float4 vectorValue, const double& newValue);
        Platform::String^ GetComponentValueAsString(Platform::String^ componentName, const WFN::float4& vectorValue);
        // Event handlers
        void OnVectorKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);
        void OnVectorPointerEntered(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnVectorPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        //void OnVectorPointerPressed(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Capture pointer input
        //void OnVectorPointerMoved(); // Update values based on movement
        //void OnVectorPointerReleased(); // Release pointer

        // General helper functions
        double GetValueFromTextBox(Platform::String^ text, double currentValue, double min, double max);

    private:
        // Scalar dependency properties
        static WUX::DependencyProperty^ m_sliderMinimum;
        static WUX::DependencyProperty^ m_sliderMaximum;

        // Scalar Controls
        WUX::Controls::Slider^ m_sliderElement;
        WUX::Controls::TextBox^ m_scalarTextBox;

        // Vector constants
        // Text boxes
        static Platform::String^ m_vectorComponentName_X;
        static Platform::String^ m_vectorComponentName_Y;
        static Platform::String^ m_vectorComponentName_Z;
        static Platform::String^ m_vectorComponentName_W;
        // Text blocks
        static Platform::String^ m_componentIdentifierName_X;
        static Platform::String^ m_componentIdentifierName_Y;
        static Platform::String^ m_componentIdentifierName_Z;
        static Platform::String^ m_componentIdentifierName_W;

        // Boolean Controls

        // Pointer to the property that defines this panel
        Engine_WinRT::PropertyCX^ m_property;
    };
}
