#pragma once
#include "Property.h"
#include <vector>
#include <map>
namespace Engine
{
    // Any classes derived from this can be represented in the debug UI as part of a 
    // SceneNode or Component. This class contains functions to add Property's that can be
    // accessed by the UI layer, It also has a name that should be defined by the inherited class
    // to be displayed in the UI 
    class SceneElement : public std::enable_shared_from_this<SceneElement>
    {
    public:
        SceneElement(std::wstring elementName);

        void RegisterBooleanProperty(const std::wstring& name,
            Property::BoolPropertyGetter boolGetter,
            Property::BoolPropertySetter boolSetter);

        void RegisterScalarProperty(const std::wstring& name,
            Property::ScalarPropertyGetter scalarGetter,
            Property::ScalarPropertySetter scalarSetter,
            float min = 0.0f,
            float max = 0.0f);

        void RegisterVectorProperty(const std::wstring& name,
            Property::VectorPropertyGetter vectorGetter,
            Property::VectorPropertySetter vectorSetter,
            const Utils::Maths::Vector4& minimum = {},
            const Utils::Maths::Vector4& maximum = {});

        Property::Ptr GetProperty(std::wstring name);

        // TODO: add functions to set and get values to avoid returning pointers.

        // Used when constructing UI. Not advisable to use otherwise
        std::vector<Property::Ptr> GetProperties();
        std::wstring GetElementName()const { return m_name; }

    private:
        void AddProperty(Property::Ptr property);

    private:
        std::map<std::wstring, Property::Ptr> m_publicProperties;
        std::wstring m_name;

        // Allows us the SceneElement class to create properties
    };
} // end namespace Engine