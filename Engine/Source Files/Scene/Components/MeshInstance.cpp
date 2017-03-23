#include "pch.h"
#include <Scene\Components\MeshInstance.h>
#include <Scene\Components\BoundingBox.h>
#include <Scene\SceneNode.h>
#include <Scene\Scene.h>
#include <Header Files\d3dclass.h>

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
        if (m_computeBounds)
        {
            auto node = GetSceneNode();
            auto bounds = node->GetComponentOfType<BoundingBox>();

            if (bounds == nullptr)
            {
                // add a bounds component if one does not exist
                // and compute the bounds for this MeshInstance
                bounds = node->AddComponent<BoundingBox>();
            }

            auto extents = bounds->GetBounds();
            auto size = extents.GetSize();
            if (size == Utils::Maths::Vector3::Zero)
            {
                // Calculate the extents
                extents = Utils::Maths::BoundingBox::CreateFromVertices(m_mesh->GetPositions());
                bounds->SetBounds(extents);
                // Since the bounds for this object has changed we need to notify the scene to
                // update the scene bounds
                node->GetScene()->NotifySceneChanged();
            }

            m_computeBounds = false;
        }
    }

    void MeshInstance::Render(ID3D11DeviceContext* deviceContext, ShaderPipeline::Ptr shaderOverride /*= nullptr*/) const
    {
        auto sceneNode = GetSceneNode();
        auto transform = sceneNode->GetWorldTransform();
        auto worldToCameraTransform = sceneNode->GetScene()->GetWorldToCameraTransform();
        auto prevWorldToCameraTransform = sceneNode->GetScene()->GetPreviousWorldToCameraTransform();

        m_objectConstants->SetData({ transform, transform * worldToCameraTransform, m_prevTransform * prevWorldToCameraTransform});
        m_objectConstants->UploadData(deviceContext);
        m_material->Render(deviceContext);
        if (shaderOverride != nullptr)
        {
            shaderOverride->UploadData(deviceContext);
        }

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
        // A meshinstance should be supplied with a valid mesh
        EngineAssert(mesh != nullptr);

        if (m_mesh != mesh)
        {
            m_mesh = mesh;
            m_computeBounds = true;
        }
    }
}