#pragma once
#include <Engine\Header Files\Scene\Public Properties\Property.h>

namespace WFN = Windows::Foundation::Numerics;

namespace Engine_WinRT
{
    public enum class PropertyType
    {
        Bool,
        Scalar,
        Vector
    };

    public ref class PropertyCX sealed
    {
    public:
        property Platform::String^ Name
        {
            Platform::String^ get()
            {
                return m_name;
            }
        }

        property PropertyType Type
        {
            PropertyType get()
            {
                return m_type;
            }
        }

        property WFN::float3 Vector
        {
            WFN::float3 get();
            void set(WFN::float3 value);
        }

        property bool Bool
        {
            bool get();
            void set(bool value);
        }

        property double Scalar
        {
            double get();
            void set(double value);
        }

        property WFN::float3 VectorMinimum
        {
            WFN::float3 get();
        }

        property WFN::float3 VectorMaximum
        {
            WFN::float3 get();
        }

        property double ScalarMinimum
        {
            double get();
        }

        property double ScalarMaximum
        {
            double get();
        }

    internal:
        PropertyCX(std::weak_ptr<Engine::Property>);
    private:

        std::weak_ptr<Engine::Property> m_nativeProperty;
        Platform::String^ m_name;
        PropertyType m_type;
    };
} // End namespace Engine_WinRT