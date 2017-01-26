#include "pch.h"
#include <RenderTargetBundle.h>
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    RenderTargetBundle::RenderTargetBundle(uint32_t width, uint32_t height, ID3D11Device* device)
        : m_width(width), m_height(height), m_device(device)
    {
        // Create depth buffer
        m_depthBuffer = std::make_shared<DepthBuffer>(m_width, m_height, 0, m_device);
    }

    void RenderTargetBundle::CreateRenderTarget(std::wstring name, DXGI_FORMAT format)
    {
        // Not allowed to create bundles greated than max size
        EngineAssert(m_count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
        EngineAssert(!m_finalised);

        // Create render target resource
        auto newRenderTarget = std::make_shared<RenderTarget>(m_width, m_height, 0, format, m_device);

        // Add render target view to array.
        m_renderTargetMap.emplace(name, newRenderTarget);
        m_renderTargetViews[m_count] = newRenderTarget->GetRTV().Get();

        // Increment count
        m_count++;
    }

    RenderTarget::Ptr RenderTargetBundle::GetRenderTarget(std::wstring name)
    {
        auto mapIt = m_renderTargetMap.find(name);
        // Ensure that the render target exists in the bundle
        EngineAssert(mapIt != m_renderTargetMap.end());

        return mapIt->second;
    }

    void RenderTargetBundle::Finalise()
    {
        // Not dangerous that it is already finalised.
        // We just want to catch silly mistakes.
        EngineAssert(!m_finalised);

        // No more render targets can be created
        m_finalised = true;
    }

    void RenderTargetBundle::SetShaderResources(ID3D11DeviceContext* deviceContext)
    {
        int textureRegister = 0;
        for (auto pair : m_renderTargetMap)
        {
            auto texture = pair.second->GetTexture();

            texture->UploadData(deviceContext, PipelineStage::Pixel, textureRegister);
            textureRegister++;
        }
    }

    ID3D11RenderTargetView* const* RenderTargetBundle::GetRenderTargetViews()
    {
        // The layout of the bundle must have been finalised already
        EngineAssert(m_finalised);

        // return array of render targets
        return m_renderTargetViews.data();
    }

} // End namespace Engine