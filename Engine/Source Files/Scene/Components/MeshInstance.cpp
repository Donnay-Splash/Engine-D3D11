#include "pch.h"
#include <Scene\Components\MeshInstance.h>
#include <Scene\SceneNode.h>
#include <Scene\Scene.h>

namespace Engine
{
    MeshInstance::MeshInstance(Component::SceneNodePtr sceneNode) : Component(sceneNode, L"Mesh Instance")
    {

    }

    void MeshInstance::Initialize(ID3D11Device* device)
    {
        // Initialize object constants buffer
        m_objectConstants = std::make_shared<ConstantBuffer<ObjectConstants>>(PipelineStage::Vertex, device);
    }

    void MeshInstance::Update(float frameTime)
    {

    }

    void MeshInstance::Render(ID3D11DeviceContext* deviceContext) const
    {
        auto sceneNode = GetSceneNode();
        auto transform = sceneNode->GetWorldTransform();
        auto worldToCameraTransform = sceneNode->GetScene()->GetWorldToCameraTransform();
        auto prevWorldToCameraTransform = sceneNode->GetScene()->GetPreviousWorldToCameraTransform();

        m_objectConstants->SetData({ transform });
        m_objectConstants->UploadData(deviceContext);
        m_material->Render(deviceContext);

        m_prevTransform = transform;

        m_mesh->Render(deviceContext);
    }

    void MeshInstance::SetMaterial(Material::Ptr material)
    {
        // TODO: Figure out how to remove elements
        // to allow for new materials to be set 
        // while removing the old one.
        m_material = material;
        
        // Add the child element to the
        // MeshInstance so that it appears in the UI.
        AddChildElement(m_material);
    }

    void MeshInstance::SetMesh(Mesh::Ptr mesh)
    {
        m_mesh = mesh;
    }
}