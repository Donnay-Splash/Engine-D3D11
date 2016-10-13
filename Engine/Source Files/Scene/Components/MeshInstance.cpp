#include "pch.h"
#include <Scene\Components\MeshInstance.h>
#include <Scene\SceneNode.h>

MeshInstance::MeshInstance(Component::SceneNodePtr sceneNode) : Component(sceneNode, "Mesh Instance")
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

    m_objectConstants->SetData({transform});
    m_objectConstants->UploadData(deviceContext);
    m_material->Render(deviceContext);

    m_mesh->Render(deviceContext);
}

void MeshInstance::SetMaterial(Material::Ptr material)
{
    m_material = material;
}

void MeshInstance::SetMesh(Mesh::Ptr mesh)
{
    m_mesh = mesh;
}