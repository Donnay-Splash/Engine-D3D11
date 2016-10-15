//
// PropertyPanel.h
// Declaration of the PropertyPanel class.
//

#pragma once
namespace WUX = Windows::UI::Xaml;

namespace WinRT_App
{
    public ref class PropertyPanel sealed : public WUX::Controls::ContentControl
    {
    public:
        PropertyPanel(Engine_WinRT::PropertyCX^ property);

    protected:
        virtual void OnApplyTemplate() override
        {
            WUX::Controls::ContentControl::OnApplyTemplate();
        }

    private:
    private:
    };
}
