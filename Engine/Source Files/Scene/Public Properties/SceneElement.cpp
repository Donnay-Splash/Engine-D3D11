#include "pch.h"
#include <Scene\Public Properties\SceneElement.h>

static uint32_t s_elementID = 0;

namespace Engine
{
    SceneElement::SceneElement(std::wstring elementName) :
        m_name(elementName), m_id(s_elementID++)
    {
    }

    void SceneElement::RegisterBooleanProperty(const std::wstring & name,
        Property::BoolPropertyGetter boolGetter,
        Property::BoolPropertySetter boolSetter)
    {
        Property::VectorPropertyGetter vectorGetter = [boolGetter]()
        {
            auto value = boolGetter();
            return Utils::Maths::Vector4(value ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
        };

        Property::VectorPropertySetter vectorSetter = [boolSetter](Utils::Maths::Vector4 value)
        {
            boolSetter(value.x > 0.0);
        };

        auto property = std::shared_ptr<Property>(new Property(name,
            PropertyType::Bool,
            vectorGetter,
            vectorSetter));

        AddProperty(property);
    }

    void SceneElement::RegisterScalarProperty(const std::wstring & name,
        Property::ScalarPropertyGetter scalarGetter,
        Property::ScalarPropertySetter scalarSetter,
        float min,
        float max)
    {
        Property::VectorPropertyGetter vectorGetter = [scalarGetter]()
        {
            auto value = scalarGetter();
            return Utils::Maths::Vector4(value, 0.0f, 0.0f, 0.0f);
        };

        Property::VectorPropertySetter vectorSetter = [scalarSetter](Utils::Maths::Vector4 value)
        {
            scalarSetter(value.x);
        };

        Utils::Maths::Vector4 vectorMin = { min, 0.0f, 0.0f, 0.0f };
        Utils::Maths::Vector4 vectorMax = { max, 0.0f, 0.0f, 0.0f };

        auto property = Property::Ptr(new Property(name,
            PropertyType::Scalar,
            vectorGetter,
            vectorSetter,
            vectorMin,
            vectorMax));

        AddProperty(property);
    }

    void SceneElement::RegisterVectorProperty(const std::wstring & name,
        Property::VectorPropertyGetter vectorGetter,
        Property::VectorPropertySetter vectorSetter,
        const Utils::Maths::Vector4 & minimum,
        const Utils::Maths::Vector4 & maximum)
    {
        auto property = Property::Ptr(new Property(name,
            PropertyType::Vector,
            vectorGetter,
            vectorSetter,
            minimum,
            maximum));

        AddProperty(property);
    }

    Property::Ptr SceneElement::GetProperty(std::wstring name)
    {
        auto it = m_publicProperties.find(name);
        if (it == m_publicProperties.end())
        {
            return nullptr;
        }
        else
        {
            return it->second;
        }
    }

    void SceneElement::AddChildElement(SceneElement::Ptr child)
    {
        m_childElements.insert(child);
        if (m_onChildElementAdded != nullptr)
        {
            m_onChildElementAdded(child, m_id);
        }
    }

    void SceneElement::AddProperty(Property::Ptr property)
    {
        if (property != nullptr)
        {
            // Two properties can not have the same name
            EngineAssert(m_publicProperties.find(property->GetName()) == m_publicProperties.end());
            m_publicProperties[property->GetName()] = property;
        }
    }

    std::vector<Property::Ptr> SceneElement::GetProperties() const
    {
        std::vector<Property::Ptr> result;
        for (auto it : m_publicProperties)
        {
            result.push_back(it.second);
        }
        return result;
    }

    std::vector<SceneElement::Ptr> SceneElement::GetChildElements() const
    {
        return std::vector<SceneElement::Ptr>(m_childElements.begin(), m_childElements.end());
    }
}