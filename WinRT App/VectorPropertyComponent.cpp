//
// VectorPropertyComponent.cpp
// Implementation of the VectorPropertyComponent class.
//

#include "pch.h"
#include "VectorPropertyComponent.h"
#include <Utils\Math\Math.h>
#include <Utils\Strings\StringHelpers.h>

using namespace WinRT_App;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Devices::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

// The Templated Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234235

VectorPropertyComponent::VectorPropertyComponent(double initialValue, double min/*= 0.0*/, double max/*= 0.0*/) :
    m_value(initialValue), m_minimum(min), m_maximum(max)
{
    DefaultStyleKey = "WinRT_App.VectorPropertyComponent";
}


/****************************************************************************
*
* Event handlers
*
****************************************************************************/
void VectorPropertyComponent::OnKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args)
{
    if (args->Key == Windows::System::VirtualKey::Enter)
    {
        auto textBox = (WUX::Controls::TextBox^)sender;
        auto componentName = textBox->Name;

        double newValue = GetValueFromTextBox(textBox->Text);
        if (newValue != Value)
        {
            Value = newValue;
        }
    }
}

void VectorPropertyComponent::OnPointerEntered(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    SetCursor(CoreCursorType::SizeWestEast);
}

void VectorPropertyComponent::OnPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // Don't think this is called if we have captured but just to be sure.
    if (!m_captured)
    {
        SetCursor(CoreCursorType::Arrow);
    }
}

void VectorPropertyComponent::OnPointerPressed(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // Capture pointer and set that it has been pressed
    auto pointer = args->Pointer;
    if (pointer->PointerDeviceType == PointerDeviceType::Mouse)
    {
        auto currentPoint = args->GetCurrentPoint(this);
        auto properties = currentPoint->Properties;
        if (properties->IsLeftButtonPressed)
        {
            m_pressed = true;
            m_captured = ((TextBlock^)sender)->CapturePointer(pointer);

            m_prevPosition = currentPoint->Position;
        }
    }

    args->Handled = true;
}

void VectorPropertyComponent::OnPointerMoved(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // If pointer pressed update value
    if (m_pressed)
    {
        auto currentPoint = args->GetCurrentPoint(this);
        auto newPosition = currentPoint->Position;
        double positionDelta = newPosition.X - m_prevPosition.X;
        // Do something with delta


        m_prevPosition = newPosition;
    }

    args->Handled = true;
}

void VectorPropertyComponent::OnPointerReleased(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // Release pointer capture and set cursor if need be
    auto pointer = args->Pointer;
    if (pointer->PointerDeviceType == PointerDeviceType::Mouse)
    {
        auto properties = args->GetCurrentPoint(this)->Properties;
        if (!properties->IsLeftButtonPressed)
        {
            m_pressed = false;
            if (m_captured)
            {
                m_captured = false;
                ((TextBlock^)sender)->ReleasePointerCapture(pointer);

                // Exited won't be called if we have the capture so we need to set cursor back to default
                SetCursor(CoreCursorType::Arrow);
            }
        }
    }

    args->Handled = true;
}

void VectorPropertyComponent::OnPointerCaptureLost(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    if (m_captured)
    {
        auto pointer = args->Pointer;
        m_captured = false;
        ((TextBlock^)sender)->ReleasePointerCapture(pointer);

        // Exited won't be called if we have the capture so we need to set cursor back to default
        SetCursor(CoreCursorType::Arrow);

        m_pressed = false;
    }
}

/****************************************************************************
*
* Helper functions
*
****************************************************************************/

double VectorPropertyComponent::GetValueFromTextBox(Platform::String^ text)
{
    // Try and convert the text value
    wchar_t* e;
    double newValue = std::wcstod(text->Data(), &e);

    bool succeeded = (errno == 0) && (e == text->End());
    if (succeeded)
    {
        if (Maximum > Minimum)
        {
            return Utils::Maths::Clamp(newValue, Minimum, Maximum);
        }
        else
        {
            return newValue;
        }
    }
    else
    {
        return Value;
    }
}

Platform::String^ VectorPropertyComponent::GetValueAsString()
{
    auto wstringValue = Utils::StringHelpers::ToWideStringWithPrecision(Value, 2, true);
    return ref new Platform::String(wstringValue.c_str());
}

void VectorPropertyComponent::SetCursor(CoreCursorType type)
{
    Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(type, 1);
}