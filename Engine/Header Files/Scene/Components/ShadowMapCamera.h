#pragma once
#include "Camera.h"
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    class ShadowMapCamera : public Camera
    {
    public:
        using Ptr = std::shared_ptr<ShadowMapCamera>;
        ShadowMapCamera(const ShadowMapCamera&) = delete;

        void Render(ID3D12GraphicsCommandList* commandList, Scene::Ptr scene, Light::Ptr light, ShaderPipeline::Ptr shadowMapPipeline);
        Utils::Maths::Matrix GetWorldToProjectedLightSpaceTransform() const { return m_worldToProjectedLightSpace; }

    private:
        ShadowMapCamera(Component::SceneNodePtr sceneNode);
        virtual void Initialize() override;

    private:
        std::shared_ptr<ConstantBuffer<ShadowMapConstants>> m_lightViewConstants;
        Utils::Maths::Matrix m_worldToProjectedLightSpace;


        friend class SceneNode;
    };
}// end namespace Engine
