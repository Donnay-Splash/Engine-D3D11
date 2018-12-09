/*  Like TextureMipView but for use with entire RenderTarget Bundles*/
#pragma once
#include "TextureMipView.h"
#include <Engine\Header Files\RenderTargetBundle.h>

namespace Engine
{
    class TextureBundleMipView
    {
    public:
        using Ptr = std::shared_ptr<TextureBundleMipView>;
        TextureBundleMipView(RenderTargetBundle::Ptr bundle, uint32_t mipLevels = 0);

        /*  Uploads the selected mip to the pixel shader. The textures will be uploaded in order starting at index 0 to
            the render target count - 1. An offset can be supplied to select the starting offset the textures are uploaded at*/
        void UploadData(ID3D11DeviceContext* deviceContext, uint32_t registerOffset = 0);
        void SetCurrentMip(uint32_t mipLevel) { m_currentMipLevel = mipLevel; }
        RenderTargetBundle::Ptr GetRenderTargetBundle() const { return m_renderTargetBundle; }

    private:
        RenderTargetBundle::Ptr m_renderTargetBundle;

        std::vector<TextureMipView::Ptr> m_mipViews;
        uint32_t m_mipLevels = 0;;
        uint32_t m_currentMipLevel = 0;
    };
}// end namespace Engine