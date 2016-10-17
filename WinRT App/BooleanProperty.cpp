//
// BooleanProperty.cpp
// Implementation of the BooleanProperty class.
//

#include "pch.h"
#include "BooleanProperty.h"

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

// The Templated Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234235

BooleanProperty::BooleanProperty(bool initialValue) :
    m_value(initialValue)
{
    DefaultStyleKey = "WinRT_App.BooleanProperty";
}

void BooleanProperty::OnApplyTemplate()
{
    WUX::Controls::ContentControl::OnApplyTemplate();

    auto checkBox = (WUX::Controls::CheckBox^)GetTemplateChild("CheckBox");
    if (checkBox == nullptr)
    {
        throw ref new Platform::FailureException("Failed to recieve CheckBox control from template");
    }
    else
    {
        checkBox->IsChecked = Value;
        checkBox->Checked += ref new WUX::RoutedEventHandler(this, &BooleanProperty::OnChecked);
        checkBox->Unchecked += ref new WUX::RoutedEventHandler(this, &BooleanProperty::OnUnchecked);
    }
}

void BooleanProperty::OnChecked(Platform::Object^ sender, WUX::RoutedEventArgs^ args)
{
    Value = true;
}

void BooleanProperty::OnUnchecked(Platform::Object^ sender, WUX::RoutedEventArgs^ args)
{
    Value = false;
}