#include "pch.h"
#include <Scene\Scene.h>
#include <Scene\Components\BoundingBox.h>

namespace Engine
{
    Scene::Scene()
    {

    }

    void Scene::Initialize()
    {
        m_rootNode = SceneNode::Create(shared_from_this(), L"Root Node", true);
        EngineAssert(m_rootNode->IsRootNode());
    }

    SceneNode::Ptr Scene::AddNode(SceneNode::Ptr parentNode /*= nullptr*/, std::wstring name /*= "Scene Node"*/)
    {
        // If no parent was specified then we attach the new node to the root of the scene
        if (parentNode == nullptr)
        {
            parentNode = m_rootNode;
        }

        // Create the new node to add to the scene
        SceneNode::Ptr newNode = SceneNode::Create(shared_from_this(), name);

        // Set the parent of the new node and add the new node as a child of the parent node.
        newNode->m_parentNode = parentNode;
        parentNode->AddChildNode(newNode);

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

} // End namespace Engine