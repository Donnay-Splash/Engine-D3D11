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

VectorProperty::VectorProperty(WFN::float3 initialValue, WFN::float3 min, WFN::float3 max) :
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

    if (componentX == nullptr || componentY == nullptr || componentZ == nullptr)
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

        // Bind to events
        componentX->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);
        componentY->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);
        componentZ->ValueChanged += ref new VectorComponentValueChangedEventHandler(this, &VectorProperty::OnComponentValueChanged);
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

    Value = currentValue;
}