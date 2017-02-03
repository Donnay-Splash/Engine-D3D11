#pragma once
#include "Component.h"
#include <Resources\Mesh.h>
#include <Resources\Material.h>
#include <Resources\ConstantBuffer.h>
#include <Resources\ConstantBufferLayouts.h>

namespace Engine
{
    class MeshInstance : public Component
    {
    public:
        using Ptr = std::shared_ptr<MeshInstance>;
        virtual void Update(float frameTime) override;
        virtual void Render(ID3D11DeviceContext* deviceContext) const override;

        void SetMesh(Mesh::Ptr mesh);
        void SetMaterial(Material::Ptr material);

        Mesh::Ptr GetMesh() const { return m_mesh; }
        Material::Ptr GetMaterial() const { return m_material; }

    protected:
        MeshInstance(Component::SceneNodePtr sceneNode);
        virtual void Initialize(ID3D11Device* device) override;
        virtual void OnSceneNodeTransformChanged(const Utils::Maths::Matrix& prevWorldTransform) override
        {
            m_prevTransform = prevWorldTransform;
        }

    private:
        Mesh::Ptr m_mesh;
        Material::Ptr m_material;
        ConstantBuffer<ObjectConstants>::Ptr m_objectConstants;
        Utils::Maths::Matrix m_prevTransform;

        friend class SceneNode;
    };
}