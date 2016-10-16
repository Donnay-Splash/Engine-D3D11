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

    DependencyProperty^ PropertyPanel::m_boundPropertyContent =
    DependencyProperty::Register("Property",
    Platform::Object::typeid,
    PropertyPanel::typeid,
    nullptr);


PropertyPanel::PropertyPanel(Engine_WinRT::PropertyCX^ property) :
    m_property(property)
{
    Content = m_property->Name;
    DefaultStyleKey = "WinRT_App.PropertyPanel";
}

void PropertyPanel::OnApplyTemplate()
{
    WUX::Controls::ContentControl::OnApplyTemplate();

    switch (m_property->Type)
    {
        case Engine_WinRT::PropertyType::Vector:
        {
            auto vectorProperty = ref new VectorProperty(m_property->Vector, m_property->VectorMinimum, m_property->VectorMaximum);
            // Bind to events
            vectorProperty->ValueChanged += ref new VectorPropertyChangedEventHandler(this, &PropertyPanel::OnVectorPropertyChanged);
            Property = vectorProperty;
            break;
        }

        case Engine_WinRT::PropertyType::Scalar:
        {   
            auto scalarProperty = ref new ScalarProperty(m_property->Scalar, m_property->ScalarMinimum, m_property->ScalarMaximum);
            // Bind to events
            scalarProperty->ValueChanged += ref new ScalarPropertyChangedEventHandler(this, &PropertyPanel::OnScalarPropertyChanged);
            Property = scalarProperty;
            break;
        }

    case Engine_WinRT::PropertyType::Bool:
    default:
        throw ref new Platform::Exception(E_INVALIDARG, "Invalid property type received");
        break;
    }
}

void PropertyPanel::OnVectorPropertyChanged(Platform::Object ^ sender, WFN::float4 newValue)
{
    m_property->Vector = newValue;
}

void PropertyPanel::OnScalarPropertyChanged(Platform::Object ^ sender, double newValue)
{
    m_property->Scalar = newValue;
}
