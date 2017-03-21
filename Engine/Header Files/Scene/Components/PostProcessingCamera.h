#pragma once
#include "Camera.h"
#include <Resources\Mesh.h>
#include <PostEffect.h>

namespace Engine
{
    // Is this a good name?
    // We don't really want it to be rendered as though it is a normal camera
    // We only want perspective calculation and render target submission
    class PostProcessingCamera : public Camera
    {
    public:
        using Ptr = std::shared_ptr<PostProcessingCamera>;
        PostProcessingCamera(const PostProcessingCamera&) = delete;

        // Given the post effect we can render to our full screen quad
        template<class T>
        void RenderPostEffect(D3DClass::Ptr d3dClass, std::shared_ptr<PostEffect<T>> postEffect)
        {
            EngineAssert(postEffect != nullptr);

            d3dClass->SetRenderTarget(m_renderTargetBundle, m_clipOffset);
            postEffect->Render(d3dClass->GetDeviceContext());
            m_screenAlignedQuad->Render(d3dClass->GetDeviceContext());
        }

        void SetClipRect(const Utils::Maths::Vector2& clipRect) { m_clipOffset = clipRect; };

    protected:
        PostProcessingCamera(Component::SceneNodePtr sceneNode);
        virtual void Initialize(ID3D11Device* device) override;

    private:
        Mesh::Ptr m_screenAlignedQuad; // Our screen aligned quad we will draw to.
        Utils::Maths::Vector2 m_clipOffset;

        friend class SceneNode;
    };
}