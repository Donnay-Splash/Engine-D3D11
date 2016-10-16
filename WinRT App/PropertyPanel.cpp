//
// PropertyPanel.cpp
// Implementation of the PropertyPanel class.
//

#include "pch.h"
#include "PropertyPanel.h"

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

DependencyProperty^ PropertyPanel::m_sliderMinimum =
DependencyProperty::Register("SliderMinimum",
    double::typeid,
    PropertyPanel::typeid,
    ref new PropertyMetadata(0.0));

DependencyProperty^ PropertyPanel::m_sliderMaximum =
DependencyProperty::Register("SliderMaximum",
    double::typeid,
    PropertyPanel::typeid,
    ref new PropertyMetadata(100.0));

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
        // If there was no min or max defined 
        // or if we have received invalid values
        // we will use the defaults instead
        if (m_property->ScalarMaximum != m_property->ScalarMinimum)
        {
            SliderMinimum = m_property->ScalarMinimum;
            SliderMaximum = m_property->ScalarMaximum;
        }
        break;

    case Engine_WinRT::PropertyType::Bool:
    default:
        throw ref new Platform::Exception(E_INVALIDARG, "Invalid property type received");
        break;
    }

    WUX::Style^ style = (WUX::Style^)Application::Current->Resources->Lookup(styleKey);
    Style = style;
}

void WinRT_App::PropertyPanel::OnSliderValueChanged(Platform::Object ^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs ^ args)
{
    m_property->Scalar = (double)args->NewValue;

    // Update text box
    if (m_textBox != nullptr)
    {
        m_textBox->Text = ref new Platform::String(std::to_wstring(m_sliderElement->Value).c_str());
    }
}

void WinRT_App::PropertyPanel::OnTextBoxKeyDown(Platform::Object ^ sender, WUX::Input::KeyRoutedEventArgs ^ args)
{
    // If enter is pressed while the text box is focussed
    // then we will unfocus the text box and set the
    // slider value to the textbox data if it is valid.
    // If it isn't valid then we will do the opposite
    // and set the textbox data to the slider val.
    if (args->Key == Windows::System::VirtualKey::Enter)
    {
        if (m_textBox != nullptr)
        {
            auto value = m_textBox->Text;
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
                    m_textBox->Text = ref new Platform::String(std::to_wstring(m_sliderElement->Value).c_str());
                }
            }
        }
    }
}

