#include "pch.h"
#include <Scene\Components\Component.h>
#include <Scene\SceneNode.h>

Component::Component(SceneNodePtr sceneNode)
{
    m_sceneNode = sceneNode;
}