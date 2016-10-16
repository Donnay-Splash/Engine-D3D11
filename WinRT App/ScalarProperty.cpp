//
// ScalarProperty.cpp
// Implementation of the ScalarProperty class.
//

#include "pch.h"
#include "ScalarProperty.h"
#include <Utils\Math\Math.h>
#include <Utils\Strings\StringHelpers.h>

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

// Scalar dependency properties
DependencyProperty^ ScalarProperty::m_sliderMinimum =
DependencyProperty::Register("SliderMinimum",
    double::typeid,
    ScalarProperty::typeid,
    ref new PropertyMetadata(0.0));

DependencyProperty^ ScalarProperty::m_sliderMaximum =
DependencyProperty::Register("SliderMaximum",
    double::typeid,
    ScalarProperty::typeid,
    ref new PropertyMetadata(100.0));

ScalarProperty::ScalarProperty(double initialValue, double min, double max) :
    m_value(initialValue)
{
    DefaultStyleKey = "WinRT_App.ScalarProperty";
    Minimum = min;
    Maximum = max;
}

void ScalarProperty::OnApplyTemplate()
{
    WUX::Controls::ContentControl::OnApplyTemplate();

    m_sliderElement = (WUX::Controls::Slider^)GetTemplateChild("Slider");
    if (m_sliderElement != nullptr)
    {
        // Set the current value
        m_sliderElement->Value = Value;

        m_sliderElement->ValueChanged +=
            ref new WUX::Controls::Primitives::RangeBaseValueChangedEventHandler(this, &ScalarProperty::OnSliderValueChanged);

        // Calculate step frequency based on range
        // We want at least 200 stops in the range
        auto range = Maximum - Minimum;
        m_sliderElement->StepFrequency = range / 200.0;

        m_scalarTextBox = (WUX::Controls::TextBox^)GetTemplateChild("TextBox");
        if (m_scalarTextBox != nullptr)
        {
            m_scalarTextBox->KeyDown += ref new Windows::UI::Xaml::Input::KeyEventHandler(this, &ScalarProperty::OnScalarTextBoxKeyDown);
        }
    }
}

void ScalarProperty::OnSliderValueChanged(Platform::Object ^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs ^ args)
{
    Value = (double)args->NewValue;

    // Update text box
    if (m_scalarTextBox != nullptr)
    {
        auto valueAsWstring = Utils::StringHelpers::ToWideStringWithPrecision(m_sliderElement->Value, 2, true);
        m_scalarTextBox->Text = ref new Platform::String(valueAsWstring.c_str());
    }
}


void ScalarProperty::OnScalarTextBoxKeyDown(Platform::Object ^ sender, WUX::Input::KeyRoutedEventArgs ^ args)
{
    // If enter is pressed while the text box is focussed
    // then we will unfocus the text box and set the
    // slider value to the textbox data if it is valid.
    // If it isn't valid then we will do the opposite
    // and set the textbox data to the slider val.
    if (args->Key == Windows::System::VirtualKey::Enter)
    {
        if (m_scalarTextBox != nullptr)
        {
            auto value = m_scalarTextBox->Text;
            bool succeeded = Focus(WUX::FocusState::Programmatic);
            // Try and convert the text value
            wchar_t* e;
            double sliderValue = std::wcstod(value->Data(), &e);

            succeeded = (errno == 0) && (e == value->End());
            if (m_sliderElement != nullptr)
            {
                if (succeeded)
                {
                    m_sliderElement->Value = sliderValue;
                }
                else
                {
                    m_scalarTextBox->Text = ref new Platform::String(std::to_wstring(m_sliderElement->Value).c_str());
                }
            }
        }
    }
}
