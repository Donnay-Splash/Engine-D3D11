//
// VectorProperty.h
// Declaration of the VectorProperty class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;
namespace WFN = Windows::Foundation::Numerics;

namespace WinRT_App
{
    public delegate void VectorPropertyChangedEventHandler(Platform::Object^ sender, WFN::float4 newValue);

    public ref class VectorProperty sealed : public WUX::Controls::ContentControl
    {
    public:
        property WFN::float4 Value
        {
        private:
            void set(WFN::float4 value)
            {
                m_value = value; 
                ValueChanged((Platform::Object^)this, value);
            }
            WFN::float4 get() { return m_value; }
        }

        event VectorPropertyChangedEventHandler^ ValueChanged;

    protected:
        virtual void OnApplyTemplate() override;

    internal:
        VectorProperty(WFN::float4 initialValue, WFN::float4 min, WFN::float4 max);

    private:
        void OnComponentValueChanged(Platform::Object^ sender, double newValue);

    private:
        // Names used to add the different vector components
        static Platform::String^ m_vectorComponentName_X;
        static Platform::String^ m_vectorComponentName_Y;
        static Platform::String^ m_vectorComponentName_Z;
        static Platform::String^ m_vectorComponentName_W;

        WFN::float4 m_value;
        WFN::float4 m_minimum;
        WFN::float4 m_maximum;
    };
}
