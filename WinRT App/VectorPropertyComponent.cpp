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
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;

// The Templated Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234235

VectorPropertyComponent::VectorPropertyComponent(double initialValue, double min, double max)
{
    DefaultStyleKey = "WinRT_App.VectorPropertyComponent";
}

Platform::String^ VectorPropertyComponent::GetValueAsString()
{
    auto wstringValue = Utils::StringHelpers::ToWideStringWithPrecision(Value, 2, true);
    return ref new Platform::String(wstringValue.c_str());
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
    Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::SizeWestEast, 1);
}

void VectorPropertyComponent::OnPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    Window::Current->CoreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 2);
}

void VectorPropertyComponent::OnPointerPressed(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // Capture pointer and set that it has been pressed
}

void VectorPropertyComponent::OnPointerMoved(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // If pointer pressed update value
}

void VectorPropertyComponent::OnPointerReleased(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args)
{
    // Release pointer capture and set cursor if need be
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