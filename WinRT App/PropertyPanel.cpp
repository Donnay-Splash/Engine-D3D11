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

// Scalar dependency properties
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

// Vector constants
// Since the values in xaml must be constant we have to ensure that these values
// match as they cannot be bound using a property
// Text boxes
Platform::String^ PropertyPanel::m_vectorComponentName_X = "Component_X";
Platform::String^ PropertyPanel::m_vectorComponentName_Y = "Component_Y";
Platform::String^ PropertyPanel::m_vectorComponentName_Z = "Component_Z";
Platform::String^ PropertyPanel::m_vectorComponentName_W = "Component_W";

// Text blocks
Platform::String^ PropertyPanel::m_componentIdentifierName_X = "Text_X";
Platform::String^ PropertyPanel::m_componentIdentifierName_Y = "Text_Y";
Platform::String^ PropertyPanel::m_componentIdentifierName_Z = "Text_Z";
Platform::String^ PropertyPanel::m_componentIdentifierName_W = "Text_W";

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

/****************************************************************************
*
* Boolean property functions
*
****************************************************************************/

/****************************************************************************
*
* Scalar property functions
*
****************************************************************************/

void PropertyPanel::ApplyScalarTemplate()
{
    m_sliderElement = (WUX::Controls::Slider^)GetTemplateChild("Slider");
    if (m_sliderElement != nullptr)
    {
        // Set the current value
        m_sliderElement->Value = m_property->Scalar;

        m_sliderElement->ValueChanged +=
            ref new WUX::Controls::Primitives::RangeBaseValueChangedEventHandler(this, &PropertyPanel::OnSliderValueChanged);

        m_scalarTextBox = (WUX::Controls::TextBox^)GetTemplateChild("TextBox");
        if (m_scalarTextBox != nullptr)
        {
            m_scalarTextBox->KeyDown += ref new Windows::UI::Xaml::Input::KeyEventHandler(this, &PropertyPanel::OnScalarTextBoxKeyDown);
        }
    }
}

void WinRT_App::PropertyPanel::OnSliderValueChanged(Platform::Object ^ sender, WUX::Controls::Primitives::RangeBaseValueChangedEventArgs ^ args)
{
    m_property->Scalar = (double)args->NewValue;

    // Update text box
    if (m_scalarTextBox != nullptr)
    {
        auto valueAsWstring = Utils::StringHelpers::ToWideStringWithPrecision(m_sliderElement->Value, 2, true);
        m_scalarTextBox->Text = ref new Platform::String(valueAsWstring.c_str());
    }
}


void WinRT_App::PropertyPanel::OnScalarTextBoxKeyDown(Platform::Object ^ sender, WUX::Input::KeyRoutedEventArgs ^ args)
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


/****************************************************************************
*
* Vector property functions
*
****************************************************************************/

// Vector functions
void PropertyPanel::ApplyVectorTemplate()
{
    // Receive the text boxes and register callbacks and assign initial values
    auto textBox_X = (WUX::Controls::TextBox^)GetTemplateChild(m_vectorComponentName_X);
    auto textBox_Y = (WUX::Controls::TextBox^)GetTemplateChild(m_vectorComponentName_Y);
    auto textBox_Z = (WUX::Controls::TextBox^)GetTemplateChild(m_vectorComponentName_Z);
    auto textBox_W = (WUX::Controls::TextBox^)GetTemplateChild(m_vectorComponentName_W);

    if (textBox_X == nullptr || textBox_Y == nullptr || textBox_Z == nullptr || textBox_W == nullptr)
    {
        throw ref new Platform::FailureException("Failed to receive all component TextBox's from template");
    }
    else
    {
        auto vectorValue = m_property->Vector;

        // Attach events to text box
        textBox_X->KeyDown += ref new WUX::Input::KeyEventHandler(this, &PropertyPanel::OnVectorKeyDown);
        textBox_Y->KeyDown += ref new WUX::Input::KeyEventHandler(this, &PropertyPanel::OnVectorKeyDown);
        textBox_Z->KeyDown += ref new WUX::Input::KeyEventHandler(this, &PropertyPanel::OnVectorKeyDown);
        textBox_W->KeyDown += ref new WUX::Input::KeyEventHandler(this, &PropertyPanel::OnVectorKeyDown);

        // Set initial value for text box
        textBox_X->Text = GetComponentValueAsString(m_vectorComponentName_X, vectorValue);
        textBox_Y->Text = GetComponentValueAsString(m_vectorComponentName_Y, vectorValue);
        textBox_Z->Text = GetComponentValueAsString(m_vectorComponentName_Z, vectorValue);
        textBox_W->Text = GetComponentValueAsString(m_vectorComponentName_W, vectorValue);
    }

    // Receive the TextBlocks and register event handlers
    auto textBlock_X = (WUX::Controls::TextBlock^)GetTemplateChild(m_componentIdentifierName_X);
    auto textBlock_Y = (WUX::Controls::TextBlock^)GetTemplateChild(m_componentIdentifierName_Y);
    auto textBlock_Z = (WUX::Controls::TextBlock^)GetTemplateChild(m_componentIdentifierName_Z);
    auto textBlock_W = (WUX::Controls::TextBlock^)GetTemplateChild(m_componentIdentifierName_W);

    if (textBlock_X == nullptr || textBlock_Y == nullptr || textBlock_Z == nullptr || textBlock_W == nullptr)
    {
        throw ref new Platform::FailureException("Failed to receive all component TextBox's from template");
    }
    else
    {
        textBlock_X->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerEntered);
        textBlock_Y->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerEntered);
        textBlock_Z->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerEntered);
        textBlock_W->PointerEntered += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerEntered);

        textBlock_X->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerExited);
        textBlock_Y->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerExited);
        textBlock_Z->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerExited);
        textBlock_W->PointerExited += ref new Windows::UI::Xaml::Input::PointerEventHandler(this, &PropertyPanel::OnVectorPointerExited);
    }
}

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

// Event handlers
void PropertyPanel::OnVectorKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args)
{
    if (args->Key == Windows::System::VirtualKey::Enter)
    {
        auto textBox = (WUX::Controls::TextBox^)sender;
        auto componentName = textBox->Name;
        double min = 0.0;
        double max = 0.0;
        double current = 0.0;
        auto vectorValue = m_property->Vector;
        
        GetValuesForComponent(componentName, vectorValue, current, min, max);

        double newValue = GetValueFromTextBox(textBox->Text, current, min, max);
        if (newValue != current)
        {
            SetValueForComponent(componentName, vectorValue, newValue);
        }
    }
}

void PropertyPanel::OnVectorPointerEntered(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::SizeWestEast, 1);
}

void PropertyPanel::OnVectorPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 2);
}

/****************************************************************************
*
* Helper functions
*
****************************************************************************/

double PropertyPanel::GetValueFromTextBox(Platform::String^ text, double currentValue, double min, double max)
{
    // Try and convert the text value
    wchar_t* e;
    double newValue = std::wcstod(text->Data(), &e);

    bool succeeded = (errno == 0) && (e == text->End());
    if (succeeded)
    {
        if (max > min)
        {
            return Utils::Maths::Clamp(newValue, min, max);
        }
        else
        {
            return newValue;
        }
    }
    else
    {
        return currentValue;
    }
}