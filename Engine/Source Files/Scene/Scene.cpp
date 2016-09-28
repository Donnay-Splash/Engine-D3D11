#include "pch.h"
#include <Scene\Scene.h>

Scene::Scene()
{

}

void Scene::Initialize()
{
    m_rootNode = SceneNode::Create(shared_from_this(), true);
    EngineAssert(m_rootNode->IsRootNode());
}

SceneNode::Ptr Scene::AddNode(SceneNode::Ptr parentNode /*= nullptr*/)
{
    // If no parent was specified then we attach the new node to the root of the scene
    if (parentNode == nullptr)
    {
        parentNode = m_rootNode;
    }

    // Create the new node to add to the scene
    SceneNode::Ptr newNode = SceneNode::Create(shared_from_this());

    // Set the parent of the new node and add the new node as a child of the parent node.
    newNode->m_parentNode = parentNode;
    parentNode->m_childNodes.push_back(newNode);

    return newNode;
}

void Scene::Update(float frameTime)
{
    m_rootNode->Update(frameTime);
}

void Scene::Render(ID3D11DeviceContext* deviceContext)
{
    m_rootNode->Render(deviceContext);
}