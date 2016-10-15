#include "pch.h"
#include <Scene\Components\Component.h>
#include <Scene\SceneNode.h>

namespace Engine
{
    Component::Component(SceneNodePtr sceneNode, std::wstring elementName) : SceneElement(elementName)
    {
        m_sceneNode = sceneNode;
    }
}