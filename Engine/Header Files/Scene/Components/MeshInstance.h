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
        virtual void Render(ID3D11DeviceContext* deviceContext, ShaderPipeline::Ptr shaderOverride = nullptr) const override;
        virtual void Update(float frameTime) override;

        void SetMesh(Mesh::Ptr mesh);
        void SetMaterial(Material::Ptr material);

        Mesh::Ptr GetMesh() const { return m_mesh; }
        Material::Ptr GetMaterial() const { return m_material; }

    protected:
        MeshInstance(Component::SceneNodePtr sceneNode);
        virtual void Initialize() override;

    private:
        Mesh::Ptr m_mesh;
        Material::Ptr m_material;
        ConstantBuffer<ObjectConstants>::Ptr m_objectConstants;
        // Not too proud. But allows us to save the previous transform
        // after rendering. 
        mutable Utils::Maths::Matrix m_prevTransform;

        /*  We need to recompute bounds when a new mesh is added to the object*/
        bool m_computeBounds = false;

        friend class SceneNode;
    };
}