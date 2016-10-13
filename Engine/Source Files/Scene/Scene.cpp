#include "pch.h"
#include <Scene\Scene.h>
#include <Scene\Components\BoundingBox.h>

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

    FireSceneNodeAddedEvent(newNode);

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

Utils::Maths::BoundingBox Scene::CalculateBoundingBoxForSceneNode(SceneNode::Ptr sceneNode)
{
    Utils::Maths::BoundingBox result;
    for (auto child : sceneNode->GetChildNodes())
    {
        auto childBounds = Scene::CalculateBoundingBoxForSceneNode(child);
        result = Utils::Maths::BoundingBox::Combine(result, childBounds);
    }

    auto boundsComponent = sceneNode->GetComponentOfType<BoundingBox>();
    if (boundsComponent)
    {
        auto bounds = boundsComponent->GetBounds();
        bounds = bounds.Transform(sceneNode->GetWorldTransform());
        result = Utils::Maths::BoundingBox::Combine(result, bounds);
    }

    return result;
}

void Scene::RegisterSceneNodeAddedCallback(SceneNodeAddedDelegate callback)
{
    EngineAssert(callback != nullptr);
    m_sceneNodeAddedCallbacks.push_back(callback);
}

void Scene::FireSceneNodeAddedEvent(SceneNode::Ptr sceneNodeAdded)
{
    for (auto callback : m_sceneNodeAddedCallbacks)
    {
        callback(sceneNodeAdded);
    }
}

void Scene::RegisterSceneNodeRemovedCallback(SceneNodeRemovedDelegate callback)
{
    EngineAssert(callback != nullptr);
    m_sceneNodeRemovedCallbacks.push_back(callback);
}

void Scene::FireSceneNodeRemovedEvent(SceneNode::Ptr sceneNodeRemoved)
{
    for (auto callback : m_sceneNodeRemovedCallbacks)
    {
        callback(sceneNodeRemoved);
    }
}