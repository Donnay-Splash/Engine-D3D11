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
    ref new PropertyMetadata(true));

    DependencyProperty^ ExpandPanel::m_cornerRadiusProperty =
    DependencyProperty::Register("CornerRadius",
    WUX::CornerRadius::typeid,
    ExpandPanel::typeid,
    nullptr);

ExpandPanel::ExpandPanel()
{
    DefaultStyleKey = "WinRT_App.ExpandPanel";
}

void WinRT_App::ExpandPanel::OnToggleClick(Platform::Object ^ sender, WUX::RoutedEventArgs ^ args)
{
    IsExpanded = !IsExpanded;
    m_toggleButton->IsChecked = IsExpanded;
    ChangeVisualState(true);
}

void WinRT_App::ExpandPanel::OnCollapseCompleted(Platform::Object ^ sender, Platform::Object ^ args)
{
    m_contentElement->Visibility = WUX::Visibility::Collapsed;
}

