#pragma once
#include <Engine\Header Files\Scene\Public Properties\Property.h>

namespace Engine_WinRT
{
    public enum class PropertyType
    {

    };

    public ref class PropertyCX sealed
    {
    public:
        property Platform::String^ Name;

    internal:
        PropertyCX(std::weak_ptr<Property>);
    private:

        std::weak_ptr<Property> m_nativeProperty;
    };
} // End namespace Engine_WinRT