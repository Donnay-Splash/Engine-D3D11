#include "pch.h"
#include "PropertyCX.h"

namespace Engine_WinRT
{
    PropertyCX::PropertyCX(std::weak_ptr<Engine::Property> nativeProperty) :
        m_nativeProperty(nativeProperty)
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            m_name = ref new Platform::String(strongProperty->GetName().c_str());
            
            // We could just cast between the types
            // but to avoid mismatch we use this long
            // winded switch statement
            switch (strongProperty->GetType())
            {
            case Engine::PropertyType::Bool:
                m_type = PropertyType::Bool;
                break;
            case Engine::PropertyType::Scalar:
                m_type = PropertyType::Scalar;
                break;
            case Engine::PropertyType::Vector:
                m_type = PropertyType::Vector;
                break;
            default:
                // Received invalid property type
                EngineAssert(false);
                break;
            }
        }
    }
} // end namespace Engine_WinRT