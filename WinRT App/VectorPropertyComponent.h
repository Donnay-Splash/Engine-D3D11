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
        VectorPropertyComponent(double initialValue, double min, double max);

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
                textBlock->PointerPressed += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerPressed);
                textBlock->PointerMoved += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerMoved);
                textBlock->PointerReleased += ref new WUX::Input::PointerEventHandler(this, &VectorPropertyComponent::OnPointerReleased);

                // Will likely also need to handle capture loss.
            }
        }
    private:
        Platform::String^ GetValueAsString();
        double GetValueFromTextBox(Platform::String^ text);

        // Event handlers
        void OnKeyDown(Platform::Object^ sender, WUX::Input::KeyRoutedEventArgs^ args);
        void OnPointerEntered(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerExited(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args);
        void OnPointerPressed(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Capture pointer input
        void OnPointerMoved(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Update values based on movement
        void OnPointerReleased(Platform::Object^ sender, WUX::Input::PointerRoutedEventArgs^ args); // Release pointer

    private:
        double m_value;
        double m_minimum;
        double m_maximum;
    };
}
