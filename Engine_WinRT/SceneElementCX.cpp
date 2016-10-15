#include "pch.h"
#include "SceneElementCX.h"

static uint32_t s_SceneElementID = 1;

namespace Engine_WinRT
{
    SceneElementCX::SceneElementCX(std::weak_ptr<Engine::SceneElement> sceneElement, std::wstring parentName, ElementType type) :
        m_nativeSceneElement(sceneElement), m_type(type)
    {
        if (auto strongElement = m_nativeSceneElement.lock())
        {
            m_name = ref new Platform::String(strongElement->GetElementName().c_str());
            m_parentName = ref new Platform::String(parentName.c_str());

            m_properties = ref new Platform::Collections::Vector<PropertyCX^>();
            for (auto prop : strongElement->GetProperties())
            {
                m_properties->Append(ref new PropertyCX(prop));
            }
        }

    }
}//end namespace Engine_WinRT