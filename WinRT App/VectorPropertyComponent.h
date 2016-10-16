//
// VectorPropertyComponent.h
// Declaration of the VectorPropertyComponent class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

namespace WinRT_App
{
    public ref class VectorPropertyComponent sealed : public WUX::Controls::ContentControl
    {
    public:
        property double Value
        {
            double get() { return m_value; }
        private:
            void set(double value) { m_value = value; }
        }

        property double Minimum
        {
            double get() { return m_minimum; }
        private:
            void set(double min) { m_minimum = min; };
        }

        property double Maximum
        {
            double get() { return m_maximum; }
        private:
            void set(double max) { m_maximum = max; }
        }

    protected:
        virtual void OnApplyTemplate() override
        {
            WUX::Controls::ContentControl::OnApplyTemplate();

            // Receive the text boxes and register callbacks and assign initial values
            auto textBox = (WUX::Controls::TextBox^)GetTemplateChild("TextBox");

            if (textBox == nullptr)
            {
                throw ref new Platform::FailureException("Failed to receive TextBox from template");
            }
            else
            {
                textBox->KeyDown += ref new WUX::Input::KeyEventHandler(this, &VectorPropertyComponent::OnKeyDown);

                // Set initial value for text box
                textBox->Text = GetValueAsString();
            }

            // Receive the TextBlocks and register event handlers
            auto textBlock = (WUX::Controls::TextBlock^)GetTemplateChild("TextBlock");

            if (textBlock == nullptr)
            {
                throw ref new Platform::FailureException("Failed to receive all component TextBox's from template");
            }
            else
            {
                textBlock->PointerEntered += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerEntered);
                textBlock->PointerExited += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerExited);
                textBlock->PointerMoved += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerMoved);
                textBlock->PointerPressed += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerPressed);
                textBlock->PointerReleased += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerReleased);
                textBlock->PointerCaptureLost += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerCaptureLost);

                // Use Release function as they do the same thing
                // Will likely also need to handle capture loss.
                textBlock->PointerCanceled += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerReleased);
            }
        }

    internal:
        VectorPropertyComponent(double initialValue, double min = 0.0, double max = 0.0);

    private:
        Platform::String^ GetValueAsString();
        double GetValueFromTextBox(Platform::String^ text);
        void SetCursor(Windows::UI::Core::CoreCursorType type);

        // Event handlers
        void OnKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);
        void OnPointerEntered(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerPressed(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Capture pointer input
        void OnPointerMoved(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Update values based on movement
        void OnPointerReleased(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Release pointer
        void OnPointerCaptureLost(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Release pointer

    private:
        double m_value;
        double m_minimum;
        double m_maximum;

        // Mouse information
        bool m_pressed = false;
        bool m_captured = false;
        Windows::Foundation::Point m_prevPosition;
    };
}
