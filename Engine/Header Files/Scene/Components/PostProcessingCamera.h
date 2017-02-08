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

            d3dClass->SetRenderTarget(m_renderTargetBundle);
            postEffect->Render(d3dClass->GetDeviceContext());
            m_screenAlignedQuad->Render(d3dClass->GetDeviceContext());
        }

    protected:
        PostProcessingCamera(Component::SceneNodePtr sceneNode);
        virtual void Initialize(ID3D11Device* device) override;

    private:
        Mesh::Ptr m_screenAlignedQuad; // Our screen aligned quad we will draw to.

        friend class SceneNode;
    };
}