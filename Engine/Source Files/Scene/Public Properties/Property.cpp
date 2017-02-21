#include "pch.h"
#include <Scene\Public Properties\Property.h>

namespace Engine
{
    Property::Property(const std::wstring& name,
        PropertyType type,
        PropertyGetter getter,
        PropertySetter setter,
        const Utils::Maths::Vector4& minimum /*= { 0.0f, 0.0f, 0.0f, 0.0f }*/,
        const Utils::Maths::Vector4& maximum /*= { 0.0f, 0.0f, 0.0f, 0.0f }*/) :
        m_name(name), m_type(type), SetValue(setter), GetValue(getter), m_minimum(minimum), m_maximum(maximum)
    {
        // A name should be provided for the property
        EngineAssert(!name.empty());
        EngineAssert(SetValue != nullptr);
        EngineAssert(GetValue != nullptr);
    }

}