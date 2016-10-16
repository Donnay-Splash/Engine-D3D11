//
// PropertyPanel.h
// Declaration of the PropertyPanel class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;
namespace WFN = Windows::Foundation::Numerics;

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
            case Engine_WinRT::PropertyType::Bool:
            default:
                throw ref new Platform::Exception(E_INVALIDARG, "Invalid property type received");
                break;
            }
        }

    private:
        // Vector functions
        void GetValuesForComponent(Platform::String^ componentName, const WFN::float4& vectorValue, double& current, double& min, double& max);
        void SetValueForComponent(Platform::String^ componentName, WFN::float4 vectorValue, const double& newValue);
        Platform::String^ GetComponentValueAsString(Platform::String^ componentName, const WFN::float4& vectorValue);

    private:
        // Vector constants
        // Text boxes
        static Platform::String^ m_vectorComponentName_X;
        static Platform::String^ m_vectorComponentName_Y;
        static Platform::String^ m_vectorComponentName_Z;
        static Platform::String^ m_vectorComponentName_W;

        // Boolean Controls

        // Pointer to the property that defines this panel
        Engine_WinRT::PropertyCX^ m_property;
    };
}
