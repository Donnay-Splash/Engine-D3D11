#include "pch.h"
#include <Scene\Public Properties\SceneElement.h>

namespace Engine
{
    SceneElement::SceneElement(std::wstring elementName) : m_name(elementName)
    {
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

    void SceneElement::AddProperty(Property::Ptr property)
    {
        if (property != nullptr)
        {
            // Two properties can not have the same name
            EngineAssert(m_publicProperties.find(property->GetName()) == m_publicProperties.end());
            m_publicProperties[property->GetName()] = property;
        }
    }

    std::vector<Property::Ptr> SceneElement::GetProperties()
    {
        std::vector<Property::Ptr> result;
        for (auto it : m_publicProperties)
        {
            result.push_back(it.second);
        }
        return result;
    }
}