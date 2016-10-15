//
// ExpandPanel.h
// Declaration of the ExpandPanel class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

// Taken from https://comentsys.wordpress.com/2015/05/30/windows-10-universal-windows-platform-expand-control/
namespace WinRT_App
{
    public ref class ExpandPanel sealed : public WUX::Controls::ContentControl
    {
    public:
        ExpandPanel();
        ExpandPanel(Engine_WinRT::SceneElementCX^ sceneElement);

        // Adds an item to the conent list.
        void AddContent(WUX::UIElement^ content);

        property Platform::Object^ HeaderContent
        {
            Platform::Object^ get() { return GetValue(m_headerContentProperty); }
            void set(Platform::Object^ object) { SetValue(m_headerContentProperty, object); }
        }

        property bool IsExpanded
        {
            bool get() { return (bool)GetValue(m_isExpandedProperty); }
            void set(bool isExpanded) { SetValue(m_isExpandedProperty, isExpanded); };
        }

        property WUX::CornerRadius CornerRadius
        {
            WUX::CornerRadius get() { return (WUX::CornerRadius)GetValue(m_cornerRadiusProperty); }
            void set(WUX::CornerRadius radius) { SetValue(m_cornerRadiusProperty, radius); }
        }



    protected:
        virtual void OnApplyTemplate() override
        {
            WUX::Controls::ContentControl::OnApplyTemplate();
            m_toggleButton = (WUX::Controls::Primitives::ToggleButton^)
                GetTemplateChild("ExpandCollapseButton");

            if (m_toggleButton != nullptr)
            {
                m_toggleButton->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &WinRT_App::ExpandPanel::OnToggleClick);
            }

            m_containerElement = (WUX::FrameworkElement^)GetTemplateChild("Container");
            if (m_containerElement != nullptr)
            {
                m_contentElement = (WUX::FrameworkElement^)GetTemplateChild("Content");
                if (m_contentElement != nullptr)
                {
                    m_collapsedState = (WUX::VisualState^)GetTemplateChild("Collapsed");
                    if ((m_collapsedState != nullptr) && (m_collapsedState->Storyboard != nullptr))
                    {
                        m_collapsedState->Storyboard->Completed += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &WinRT_App::ExpandPanel::OnCollapseCompleted);
                    }
                }
                m_headerElement = (WUX::FrameworkElement^)GetTemplateChild("Header");
            }
            ChangeVisualState(false);
        }

    private:
        void ChangeVisualState(bool useTransition)
        {
            if (IsExpanded)
            {
                if (m_contentElement != nullptr)
                {
                    m_contentElement->Visibility = WUX::Visibility::Visible;
                }
                WUX::VisualStateManager::GoToState(this, "Expanded", useTransition);
            }
            else
            {
                WUX::VisualStateManager::GoToState(this, "Collapsed", useTransition);
                m_collapsedState = (WUX::VisualState^)GetTemplateChild("Collapsed");
                if (m_collapsedState == nullptr)
                {
                    if (m_contentElement != nullptr)
                    {
                        m_contentElement->Visibility = WUX::Visibility::Collapsed;
                    }
                }
            }

        }

        void OnToggleClick(Platform::Object^ sender, WUX::RoutedEventArgs^ args);
        void OnCollapseCompleted(Platform::Object^ sender, Platform::Object^ args);

        void InitializeResources();

    private:
        static WUX::DependencyProperty^ m_headerContentProperty;
        static WUX::DependencyProperty^ m_isExpandedProperty;
        static WUX::DependencyProperty^ m_cornerRadiusProperty;

        Windows::UI::Xaml::VisualState^ m_collapsedState;
        Windows::UI::Xaml::Controls::Primitives::ToggleButton^ m_toggleButton;
        WUX::FrameworkElement^ m_contentElement;
        WUX::FrameworkElement^ m_containerElement;
        WUX::FrameworkElement^ m_headerElement;

        WUX::Controls::StackPanel^ m_contentPanel;
    };
}
