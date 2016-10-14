//
// CustomTitleBar.xaml.cpp
// Implementation of the CustomTitleBar class
//

#include "pch.h"
#include "CustomTitleBar.xaml.h"

using namespace WinRT_App;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236
DependencyProperty^ CustomTitleBar::IsChromelessProperty =
DependencyProperty::Register("IsChromeless",
    bool::typeid,
    CustomTitleBar::typeid,
    ref new PropertyMetadata(false, ref new PropertyChangedCallback(&CustomTitleBar::OnIsChromelessChanged)));

CustomTitleBar::CustomTitleBar()
{
    InitializeComponent();
}

