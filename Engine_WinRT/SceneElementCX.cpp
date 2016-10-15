#include "pch.h"
#include "SceneElementCX.h"

namespace Engine_WinRT
{
    SceneElementCX::SceneElementCX(std::weak_ptr<SceneElement> sceneElement) :
        m_nativeSceneElement(sceneElement)
    {
        if (auto strongElement = m_nativeSceneElement.lock())
        {
            m_name = ref new Platform::String(strongElement->GetElementName().c_str());

            for (auto prop : strongElement->GetProperties())
            {
                m_properties->Append(ref new PropertyCX(prop));
            }
        }

    }
}//end namespace Engine_WinRT