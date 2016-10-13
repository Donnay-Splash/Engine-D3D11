#include "pch.h"
#include "SceneElementCX.h"

namespace Engine_WinRT
{
    SceneElementCX::SceneElementCX(std::weak_ptr<SceneElement> sceneElement) :
        m_nativeSceneElement(sceneElement)
    {

    }
}//end namespace Engine_WinRT