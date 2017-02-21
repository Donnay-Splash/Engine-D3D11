#include "pch.h"
#include "PropertyCX.h"
#include <WindowsNumerics.h>

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

    WFN::float3 PropertyCX::Vector::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // Make sure that we have the render lock at this point
            auto value = strongProperty->GetValue();
            return WFN::float3(value.x, value.y, value.z);
        }
        return WFN::float3();
    }

    void PropertyCX::Vector::set(WFN::float3 value)
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            Utils::Maths::Vector4 engineValue = { value.x, value.y, value.z, 0.0f };
            strongProperty->SetValue(engineValue);
        }
    }

    double PropertyCX::Scalar::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // Make sure that we have the render lock at this point
            auto value = strongProperty->GetValue();
            return value.x;
        }
        return 0.0;
    }

    void PropertyCX::Scalar::set(double value)
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            Utils::Maths::Vector4 engineValue;
            engineValue.x = static_cast<float>(value);
            strongProperty->SetValue(engineValue);
        }
    }

    bool PropertyCX::Bool::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // Make sure that we have the render lock at this point
            auto value = strongProperty->GetValue();
            return value.x > 0.0f;
        }
        return false;
    }

    void PropertyCX::Bool::set(bool value)
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            Utils::Maths::Vector4 engineValue;
            engineValue.x = value ? 1.0f : 0.0f;
            strongProperty->SetValue(engineValue);
        }
    }

    WFN::float3 PropertyCX::VectorMinimum::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            auto v = strongProperty->GetMinimum();
            return WFN::float3(v.x, v.y, v.z);
        }

        return WFN::float3(0.0f);
    }

    WFN::float3 PropertyCX::VectorMaximum::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            auto v = strongProperty->GetMaximum();
            return WFN::float3(v.x, v.y, v.z);
        }

        return WFN::float3(0.0f);
    }

    double PropertyCX::ScalarMinimum::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            auto v = strongProperty->GetMinimum();
            return v.x;
        }
        return 0.0f;
    }

    double PropertyCX::ScalarMaximum::get()
    {
        if (auto strongProperty = m_nativeProperty.lock())
        {
            // TODO: Make sure we have the render lock at this point
            auto v = strongProperty->GetMaximum();
            return v.x;
        }
        return 0.0f;
    }

} // end namespace Engine_WinRT