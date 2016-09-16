#include "pch.h"
#include "Component.h"
#include "SceneNode.h"

Component::Component(SceneNodePtr sceneNode)
{
    m_sceneNode = sceneNode;
}