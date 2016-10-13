#include "pch.h"
#include <Scene\Components\Component.h>
#include <Scene\SceneNode.h>

Component::Component(SceneNodePtr sceneNode, std::string elementName) : SceneElement(elementName)
{
    m_sceneNode = sceneNode;
}