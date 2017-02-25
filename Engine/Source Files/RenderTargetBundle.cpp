#include "pch.h"
#include <RenderTargetBundle.h>
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    RenderTargetBundle::RenderTargetBundle(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t arraySize/*=1*/, uint32_t mipLevels/* = 1*/)
        : m_width(width), m_height(height), m_device(device), m_arraySize(arraySize), m_mipLevels(mipLevels)
    {
        // Create depth buffer
        m_depthBuffer = std::make_shared<DepthBuffer>(m_width, m_height, m_arraySize, TextureCreationFlags::BindShaderResource, m_device);
        m_bundleSampler = std::make_shared<Sampler>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);

        if (m_mipLevels < 1) m_mipLevels = 1;

        m_renderTargetViews.resize(m_mipLevels);
    }

    void RenderTargetBundle::CreateRenderTarget(std::wstring name, DXGI_FORMAT format)
    {
        // Not allowed to create bundles greated than max size
        EngineAssert(m_count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
        EngineAssert(!m_finalised);

        uint32_t textureFlags = TextureCreationFlags::BindRenderTarget | TextureCreationFlags::BindShaderResource;
        if (m_mipLevels > 1) textureFlags |= TextureCreationFlags::GenerateMIPs;

        // Create texture resource for render targets
        auto texture = Texture::CreateTextureArray(nullptr, m_width, m_height, m_arraySize, textureFlags, format, m_device);
        // Generate render targets to draw to each of the mip levels
        for (uint32_t mip = 0; mip < m_mipLevels; mip++)
        {
            auto newRenderTarget = std::make_shared<RenderTarget>(texture, 0, mip, 0, m_device);
            // Add render target view to array.
            m_renderTargets.push_back({ name, newRenderTarget });
            m_renderTargetViews[mip][m_count] = newRenderTarget->GetRTV().Get();
        }

        // Increment count
        m_count++;
    }

    RenderTarget::Ptr RenderTargetBundle::GetRenderTarget(std::wstring name)
    {
        auto it = std::find_if(m_renderTargets.begin(), m_renderTargets.end(), [name](const BundleElement& element)
        {
            return name == element.Name;
        });

        // Ensure that the render target exists in the bundle
        EngineAssert(it != m_renderTargets.end());

        return it->RenderTarget;
    }

    void RenderTargetBundle::Clear(ID3D11DeviceContext* deviceContext)
    {
        EngineAssert(m_finalised);

        // Can add custom colour later
        float color[4]{ 1.0f, 0.0f, 0.0f, 1.0f };

        for (uint32_t i = 0; i < m_count; i++)
        {
            deviceContext->ClearRenderTargetView(m_renderTargetViews[0][i], color);
        }

        deviceContext->ClearDepthStencilView(m_depthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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
        // Clear render target state
        ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullViews, nullptr);

        int textureRegister = 0;
        for (auto pair : m_renderTargets)
        {
            auto texture = pair.RenderTarget->GetTexture();

            texture->UploadData(deviceContext, PipelineStage::Pixel, textureRegister);
            textureRegister++;
        }

        m_depthBuffer->GetTexture()->UploadData(deviceContext, PipelineStage::Pixel, textureRegister);
        m_bundleSampler->UploadData(deviceContext, 0);
    }

    void RenderTargetBundle::SetTargetMip(uint32_t targetMip)
    {
        EngineAssert(targetMip < m_mipLevels);
        m_targetMip = targetMip;
    }

    ID3D11RenderTargetView* const* RenderTargetBundle::GetRenderTargetViews()
    {
        // The layout of the bundle must have been finalised already
        EngineAssert(m_finalised);

        // return array of render targets
        return m_renderTargetViews[m_targetMip].data();
    }

} // End namespace Engine