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

        property Platform::Object^ Property
        {
            Platform::Object^ get() { return GetValue(m_boundPropertyContent); }
        private:
            void set(Platform::Object^ object) { SetValue(m_boundPropertyContent, object); }
        }

    protected:
        virtual void OnApplyTemplate() override;

    private:
        void OnVectorPropertyChanged(Platform::Object^ sender, WFN::float4 newValue);
        void OnScalarPropertyChanged(Platform::Object^ sender, double newValue);
        void OnBooleanPropertyChanged(Platform::Object^ sender, bool newValue);

    private:
        // Bound to a content presenter to display the given property
        static WUX::DependencyProperty^ m_boundPropertyContent;
        WUX::FrameworkElement^ m_propertyElement;

        // Pointer to the property that defines this panel
        Engine_WinRT::PropertyCX^ m_property;
    };
}
