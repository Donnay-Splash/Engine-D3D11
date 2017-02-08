#pragma once
#include "Property.h"
#include <vector>
#include <map>
#include <set>

namespace Engine
{
    // Any classes derived from this can be represented in the debug UI as part of a 
    // SceneNode or Component. This class contains functions to add Property's that can be
    // accessed by the UI layer, It also has a name that should be defined by the inherited class
    // to be displayed in the UI 
    class SceneElement : public std::enable_shared_from_this<SceneElement>
    {
    public:
        using ChildElementsChangedEventHandler = std::function<void(std::shared_ptr<SceneElement>, uint32_t)>;
        using Ptr = std::shared_ptr<SceneElement>;

        SceneElement(std::wstring elementName);

        void SetChildAddedCallback(ChildElementsChangedEventHandler childAddedEventHandler) { m_onChildElementAdded = childAddedEventHandler; }
        void SetChildRemovedCallback(ChildElementsChangedEventHandler childRemovedEventHandler) { m_onChildElementRemoved = childRemovedEventHandler; }

        Property::Ptr GetProperty(std::wstring name);
        std::wstring GetElementName()const { return m_name; }
        // Used when constructing UI. Not advisable to use otherwise
        std::vector<Property::Ptr> GetProperties() const;
        std::vector<SceneElement::Ptr> GetChildElements() const;
        uint32_t GetID() const { return m_id; }

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

        void AddChildElement(SceneElement::Ptr childElement);

    private:
        void AddProperty(Property::Ptr property);

    private:
        std::map<std::wstring, Property::Ptr> m_publicProperties;
        std::wstring m_name;

        ChildElementsChangedEventHandler m_onChildElementAdded;
        ChildElementsChangedEventHandler m_onChildElementRemoved;

        const uint32_t m_id;

        // Set avoids adding duplicate elements
        std::set<SceneElement::Ptr> m_childElements;
    };
} // end namespace Engine