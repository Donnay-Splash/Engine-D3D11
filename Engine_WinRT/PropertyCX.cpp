#include "pch.h"
#include "PropertyCX.h"

namespace Engine_WinRT
{
    PropertyCX::PropertyCX(std::weak_ptr<Property> nativeProperty) :
        m_nativeProperty(nativeProperty)
    {

    }
} // end namespace Engine_WinRT