//
// PropertyPanel.cpp
// Implementation of the PropertyPanel class.
//

#include "pch.h"
#include "PropertyPanel.h"
#include <Utils\Math\Math.h>
#include <Utils\Strings\StringHelpers.h>

using namespace WinRT_App;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

// The Templated Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234235

// Vector constants
// Since the values in xaml must be constant we have to ensure that these values
// match as they cannot be bound using a property
// Text boxes
Platform::String^ PropertyPanel::m_vectorComponentName_X = "Component_X";
Platform::String^ PropertyPanel::m_vectorComponentName_Y = "Component_Y";
Platform::String^ PropertyPanel::m_vectorComponentName_Z = "Component_Z";
Platform::String^ PropertyPanel::m_vectorComponentName_W = "Component_W";

PropertyPanel::PropertyPanel(Engine_WinRT::PropertyCX^ property) :
    m_property(property)
{
    Content = m_property->Name;
    // Select style depending on property type
    Platform::String^ styleKey;
    switch (m_property->Type)
    {
    case Engine_WinRT::PropertyType::Vector:
        styleKey = "VectorPropertyPanelStyle";
        break;
    case Engine_WinRT::PropertyType::Scalar:
        styleKey = "ScalarPropertyPanelStyle";
        break;

    case Engine_WinRT::PropertyType::Bool:
    default:
        throw ref new Platform::Exception(E_INVALIDARG, "Invalid property type received");
        break;
    }

    WUX::Style^ style = (WUX::Style^)Application::Current->Resources->Lookup(styleKey);
    Style = style;
}

/****************************************************************************
*
* Boolean property functions
*
****************************************************************************/

/****************************************************************************
*
* Vector property functions
*
****************************************************************************/

void PropertyPanel::GetValuesForComponent(Platform::String^ componentName, const WFN::float4& vectorValue, double& current, double& min, double& max)
{
    auto vectorMax = m_property->VectorMaximum;
    auto vectorMin = m_property->VectorMinimum;

    if (componentName == m_vectorComponentName_X)
    {
        min = vectorMin.x;
        max = vectorMax.x;
        current = vectorValue.x;
    }
    else if (componentName == m_vectorComponentName_Y)
    {
        min = vectorMin.y;
        max = vectorMax.y;
        current = vectorValue.y;
    }
    else if (componentName == m_vectorComponentName_Z)
    {
        min = vectorMin.z;
        max = vectorMax.z;
        current = vectorValue.z;
    }
    else if (componentName == m_vectorComponentName_W)
    {
        min = vectorMin.w;
        max = vectorMax.w;
        current = vectorValue.w;
    }
    else
    {
        throw ref new Platform::InvalidArgumentException("Received invalid component name from TextBox");
    }
}

void PropertyPanel::SetValueForComponent(Platform::String^ componentName, WFN::float4 vectorValue, const double& newValue)
{
    if (componentName == m_vectorComponentName_X)
    {
        vectorValue.x = newValue;
    }
    else if (componentName == m_vectorComponentName_Y)
    {
        vectorValue.y = newValue;
    }
    else if (componentName == m_vectorComponentName_Z)
    {
        vectorValue.z = newValue;
    }
    else if (componentName == m_vectorComponentName_W)
    {
        vectorValue.w = newValue;
    }
    else
    {
        throw ref new Platform::InvalidArgumentException("Received invalid component name from TextBox");
    }

    m_property->Vector = vectorValue;
}

Platform::String^ PropertyPanel::GetComponentValueAsString(Platform::String^ componentName, const WFN::float4& vectorValue)
{
    double value = 0.0;
    if (componentName == m_vectorComponentName_X)
    {
        value = vectorValue.x;
    }
    else if (componentName == m_vectorComponentName_Y)
    {
        value = vectorValue.y;
    }
    else if (componentName == m_vectorComponentName_Z)
    {
        value = vectorValue.z;
    }
    else if (componentName == m_vectorComponentName_W)
    {
        value = vectorValue.w;
    }
    else
    {
        throw ref new Platform::InvalidArgumentException("Received invalid component name from TextBox");
    }

    auto wstringValue = Utils::StringHelpers::ToWideStringWithPrecision(value, 2, true);
    return ref new Platform::String(wstringValue.c_str());
}

/****************************************************************************
*
* Helper functions
*
****************************************************************************/

