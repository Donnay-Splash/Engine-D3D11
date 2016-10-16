//
// VectorProperty.cpp
// Implementation of the VectorProperty class.
//

#include "pch.h"
#include "VectorProperty.h"

using namespace WinRT_App;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

Platform::String^ VectorProperty::m_vectorComponentName_X = "Component_X";
Platform::String^ VectorProperty::m_vectorComponentName_Y = "Component_Y";
Platform::String^ VectorProperty::m_vectorComponentName_Z = "Component_Z";
Platform::String^ VectorProperty::m_vectorComponentName_W = "Component_W";

VectorProperty::VectorProperty(WFN::float4 initialValue, WFN::float4 min, WFN::float4 max) :
    m_value(initialValue), m_minimum(min), m_maximum(max)
{
    DefaultStyleKey = "WinRT_App.VectorProperty";
}

void VectorProperty::OnApplyTemplate()
{
    WUX::Controls::ContentControl::OnApplyTemplate();

    auto componentX = (VectorPropertyComponent^)GetTemplateChild(m_vectorComponentName_X);
    auto componentY = (VectorPropertyComponent^)GetTemplateChild(m_vectorComponentName_Y);
    auto componentZ = (VectorPropertyComponent^)GetTemplateChild(m_vectorComponentName_Z);
    auto componentW = (VectorPropertyComponent^)GetTemplateChild(m_vectorComponentName_W);

    if (componentX == nullptr || componentY == nullptr || componentZ == nullptr || componentW == nullptr)
    {
        throw ref new Platform::FailureException("Failed to receive all components from template");
    }
    else
    {
        auto currentValue = Value;
        // Set initial values
        componentX->Maximum = m_maximum.x;
        componentX->Minimum = m_minimum.x;
        componentX->Value = currentValue.x;

        componentY->Maximum = m_maximum.y;
        componentY->Minimum = m_minimum.y;
        componentY->Value = currentValue.y;

        componentZ->Maximum = m_maximum.z;
        componentZ->Minimum = m_minimum.z;
        componentZ->Value = currentValue.z;

        componentW->Maximum = m_maximum.w;
        componentW->Minimum = m_minimum.w;
        componentW->Value = currentValue.w;
        
        
        // Bind to events
        componentX->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);
        componentY->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);
        componentZ->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);
        componentW->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);

    }
}

void VectorProperty::OnComponentValueChanged(Platform::Object^ sender, double newValue)
{
    auto component = (VectorPropertyComponent^)sender;
    auto componentName = component->Name;
    auto currentValue = Value;
    if (componentName == m_vectorComponentName_X)
    {
        currentValue.x = newValue;
    }
    else if (componentName == m_vectorComponentName_Y)
    {
        currentValue.y = newValue;

    }
    else if (componentName == m_vectorComponentName_Z)
    {
        currentValue.z = newValue;

    }
    else if (componentName == m_vectorComponentName_W)
    {
        currentValue.w = newValue;

    }

    Value = currentValue;
}