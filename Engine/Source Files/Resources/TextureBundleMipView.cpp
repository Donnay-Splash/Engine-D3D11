#include "pch.h"
#include <Header Files\Resources\TextureBundleMipView.h>
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    TextureBundleMipView::TextureBundleMipView(RenderTargetBundle::Ptr bundle, uint32_t mipLevels/* = 0*/) :
        m_mipLevels(mipLevels), m_currentMipLevel(0), m_renderTargetBundle(bundle)
    {
        EngineAssert(m_renderTargetBundle != nullptr);
        EngineAssert(m_mipLevels <= m_renderTargetBundle->GetMipLevels());

        for (uint32_t i = 0; i < m_renderTargetBundle->RenderTargetCount(); i++)
        {
            auto texture = m_renderTargetBundle->GetRenderTarget(i)->GetTexture();
            auto mipView = std::make_shared<TextureMipView>(texture, m_mipLevels);
            m_mipViews.push_back(mipView);
            m_mipLevels = mipView->GetMipCount();
        }
    }

    void TextureBundleMipView::UploadData(ID3D11DeviceContext* context, uint32_t registerOffset /*= 0*/)
    {
        for (auto mipView : m_mipViews)
        {
            mipView->SetCurrentMip(m_currentMipLevel);
            mipView->UploadData(context, PipelineStage::Pixel, registerOffset++);
        }
    }

}// end namespace engine
