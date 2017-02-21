//
// VectorProperty.h
// Declaration of the VectorProperty class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;
namespace WFN = Windows::Foundation::Numerics;

namespace WinRT_App
{
    public delegate void VectorPropertyChangedEventHandler(Platform::Object^ sender, WFN::float3 newValue);

    public ref class VectorProperty sealed : public WUX::Controls::ContentControl
    {
    public:
        event VectorPropertyChangedEventHandler^ ValueChanged;

    protected:
        virtual void OnApplyTemplate() override;

    internal:
        VectorProperty(WFN::float3 initialValue, WFN::float3 min, WFN::float3 max);

    private:
        property WFN::float3 Value
        {
            void set(WFN::float3 value)
            {
                m_value = value; 
                ValueChanged((Platform::Object^)this, value);
            }
            WFN::float3 get() { return m_value; }
        }

        void OnComponentValueChanged(Platform::Object^ sender, double newValue);

    private:
        // Names used to add the different vector components
        static Platform::String^ m_vectorComponentName_X;
        static Platform::String^ m_vectorComponentName_Y;
        static Platform::String^ m_vectorComponentName_Z;

        WFN::float3 m_value;
        WFN::float3 m_minimum;
        WFN::float3 m_maximum;
    };
}
