//
// ExpandPanel.cpp
// Implementation of the ExpandPanel class.
//

#include "pch.h"
#include "ExpandPanel.h"

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

    DependencyProperty^ ExpandPanel::m_headerContentProperty = 
    DependencyProperty::Register("HeaderContent", 
    Platform::Object::typeid,
    ExpandPanel::typeid,
    nullptr);

    DependencyProperty^ ExpandPanel::m_isExpandedProperty =
    DependencyProperty::Register("IsExpanded",
    bool::typeid,
    ExpandPanel::typeid,
    ref new PropertyMetadata(false));

    DependencyProperty^ ExpandPanel::m_cornerRadiusProperty =
    DependencyProperty::Register("CornerRadius",
    WUX::CornerRadius::typeid,
    ExpandPanel::typeid,
    nullptr);

ExpandPanel::ExpandPanel()
{
    InitializeResources();
}

ExpandPanel::ExpandPanel(Engine_WinRT::SceneElementCX ^ sceneElement)
{
    InitializeResources();

    HeaderContent = sceneElement->Name;
    for (auto property : sceneElement->Properties)
    {
        auto propertyPanel = ref new PropertyPanel(property);
        AddContent(propertyPanel);
    }

    // Get's internal elements. 
    // Construct new expand panel for each element adding them to this panel.
    // Get Internal Elements
    // For each element
        // Internal panel = ref new ExpandPanel(internal element)
        // AddContent(Internal Panel)
}

void ExpandPanel::AddContent(WUX::UIElement^ content)
{
    // Good idea but looks ugly

    //auto border = ref new Border();
    //border->HorizontalAlignment = WUX::HorizontalAlignment::Center;
    //border->VerticalAlignment = WUX::VerticalAlignment::Center;
    //border->BorderThickness = WUX::Thickness(4.0);
    //border->BorderBrush = ref new WUX::Media::SolidColorBrush(Windows::UI::Colors::DarkGray);
    //border->Margin = WUX::Thickness(10, 2, 10, 2);
    //border->Width = 40.0;
    //border->Height = 0.5;
    //// Add border to separate properties
    //if (m_contentPanel->Children->Size != 0)
    //{
    //    m_contentPanel->Children->Append(border);
    //}
    m_contentPanel->Children->Append(content);
}

void ExpandPanel::OnToggleClick(Platform::Object ^ sender, WUX::RoutedEventArgs ^ args)
{
    IsExpanded = !IsExpanded;
    m_toggleButton->IsChecked = IsExpanded;
    if (!contentSet)
    {
        Content = m_contentPanel;
        contentSet = true;
    }
    ChangeVisualState(true);
}

void ExpandPanel::OnCollapseCompleted(Platform::Object ^ sender, Platform::Object ^ args)
{
    m_contentElement->Visibility = WUX::Visibility::Collapsed;
}

void WinRT_App::ExpandPanel::InitializeResources()
{
    DefaultStyleKey = "WinRT_App.ExpandPanel";
    m_contentPanel = ref new StackPanel();
    m_contentPanel->Orientation = WUX::Controls::Orientation::Vertical;
}

