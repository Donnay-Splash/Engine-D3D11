#pragma once
#include <functional>
#include <memory>

namespace Engine
{
    // forward declaration
    class SceneElement;

    enum class PropertyType
    {
        Bool, // 1 element
        Scalar, // 1 element
        Vector // 3 elements
        //Color // 4 elements
    };

    class Property
    {
    public:
        using BoolPropertySetter = std::function<void(bool)>;
        using BoolPropertyGetter = std::function<bool()>;

        using ScalarPropertySetter = std::function<void(float)>;
        using ScalarPropertyGetter = std::function<float()>;

        using VectorPropertySetter = std::function<void(Utils::Maths::Vector3)>;
        using VectorPropertyGetter = std::function<Utils::Maths::Vector3()>;

        using PropertySetter = std::function<void(Utils::Maths::Vector4)>;
        using PropertyGetter = std::function<Utils::Maths::Vector4()>;

        using Ptr = std::shared_ptr<Property>;

        const PropertySetter SetValue;
        const PropertyGetter GetValue;
        std::wstring GetName() const { return m_name; }
        Utils::Maths::Vector4 GetMaximum() const { return m_maximum; }
        Utils::Maths::Vector4 GetMinimum() const { return m_minimum; }
        PropertyType GetType() const { return m_type; }

    private:
        Property(const std::wstring& name,
            PropertyType type,
            PropertyGetter getter,
            PropertySetter setter,
            const Utils::Maths::Vector4& minimum = {},
            const Utils::Maths::Vector4& maximum = {});

    private:
        std::wstring m_name;
        Utils::Maths::Vector4 m_maximum;
        Utils::Maths::Vector4 m_minimum;
        PropertyType m_type;

        friend class SceneElement;
    };
} // end namespace Engine


    // Old implementation kept for reference
//    using CustomPropertySetter = std::function<void(PropertyType)>;
//    using CustomPropertyGetter = std::function<PropertyType()>;
//public:
//    using Ptr = std::shared_ptr<Property<PropertyType>>;
//    Property(const Property&) = delete;
//    // Constructor for property giving pointer to data member. No defined range.
//    Property(std::string propertyName, std::shared_ptr<PropertyType> dataPtr);
//    // Constructor for property giving pointer to data member. With Defined range.
//    Property(std::string propertyName, std::shared_ptr<PropertyType> dataPtr, const PropertyType& minimum, const PropertyType& maximum);
//    // Constructor for property giving custom setters and getters with no defined range.
//    // We do not require the pointer to the data as we presume that this is handled and 
//    // stored externally with the functions. This allows conversion from one data representation to another.
//    Property(std::string propertyName, CustomPropertySetter customSetter, CustomPropertyGetter customGetter);
//    // Constructor for property giving custom setters and getters with defined range.
//    // We do not require the pointer to the data as we presume that this is handled and 
//    // stored externally with the functions. This allows conversion from one data representation to another.
//    Property(std::string propertyName, CustomPropertySetter customSetter, CustomPropertyGetter customGetter, const PropertyType& minimum, const PropertyType& maximum);
//    ~Property() {};
//
//    CustomPropertySetter SetValue;
//    CustomPropertyGetter GetValue;
//
//private:
//    std::string m_propertyName;
//    PropertyType m_minimum;
//    PropertyType m_maximum;
//
//    std::weak_ptr<PropertyType> m_dataPtr;

//#include "Property.inl"
