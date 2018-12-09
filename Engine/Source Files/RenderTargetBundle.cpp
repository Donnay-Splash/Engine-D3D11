#include "pch.h"
#include <RenderTargetBundle.h>
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    RenderTargetBundle::RenderTargetBundle(uint32_t width, uint32_t height, uint32_t arraySize/*=1*/, uint32_t mipLevels/* = 1*/, bool depthEnabled /*= true*/)
        : m_width(width), m_height(height), m_arraySize(arraySize), m_mipLevels(mipLevels)
    {
        // Create depth buffer
        if (depthEnabled)
        {
            m_depthBuffer = std::make_shared<DepthBuffer>(m_width, m_height, m_arraySize, TextureCreationFlags::BindShaderResource);
        }

        m_bundleSampler = std::make_shared<Sampler>(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);

        if (m_mipLevels < 1) m_mipLevels = 1;

        m_renderTargetViews.resize(m_mipLevels);
    }

    void RenderTargetBundle::CreateRenderTarget(std::wstring name, DXGI_FORMAT format, const Utils::Maths::Color& clearColor /*= {}*/)
    {
        // Not allowed to create bundles greated than max size
        EngineAssert(m_count < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT);
        EngineAssert(!m_finalised);

        uint32_t textureFlags = TextureCreationFlags::BindRenderTarget | TextureCreationFlags::BindShaderResource;
        if (m_mipLevels > 1) textureFlags |= TextureCreationFlags::GenerateMIPs;

        // Create texture resource for render targets
        auto texture = Texture::CreateTextureArray(nullptr, m_width, m_height, m_arraySize, textureFlags, format);

        BundleElement newBundleElement;
        newBundleElement.ClearColour = clearColor;
        newBundleElement.Name = name;
        // Generate render targets to draw to each of the mip levels
        for (uint32_t mip = 0; mip < m_mipLevels; mip++)
        {
            auto newRenderTarget = std::make_shared<RenderTarget>(texture, 0, mip, 0);
            // Push this render target into the vector.
            newBundleElement.RenderTargets.push_back(newRenderTarget);
            // Add render target view to array.
            m_renderTargetViews[mip][m_count] = newRenderTarget->GetRTV().Get();
        }
        m_renderTargets.push_back(newBundleElement);

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

        // Return the render target for the top level mip
        return it->RenderTargets[0];
    }

    RenderTarget::Ptr RenderTargetBundle::GetRenderTarget(uint32_t index)
    {
        EngineAssert(index < m_count && index >= 0);
        // Return the render target for the top level mip
        return m_renderTargets[index].RenderTargets[0];
    }

    void RenderTargetBundle::Clear(ID3D11DeviceContext* deviceContext)
    {
        EngineAssert(m_finalised);

        // Can add custom colour later

        for (uint32_t i = 0; i < m_count; i++)
        {
            auto c = m_renderTargets[i].ClearColour;
            float color[4]{ c.x , c.y, c.z, c.w };
            deviceContext->ClearRenderTargetView(m_renderTargetViews[0][i], color);
        }

        if (m_depthBuffer != nullptr)
        {
            deviceContext->ClearDepthStencilView(m_depthBuffer->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        }
    }

    void RenderTargetBundle::Finalise()
    {
        // Not dangerous that it is already finalised.
        // We just want to catch silly mistakes.
        EngineAssert(!m_finalised);

        // No more render targets can be created
        m_finalised = true;
    }

    void RenderTargetBundle::SetShaderResources(ID3D11DeviceContext* deviceContext, uint32_t registerOffset /*= 0*/)
    {
        // Clear render target state
        ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullViews, nullptr);

        for (auto pair : m_renderTargets)
        {
            auto texture = pair.RenderTargets[0]->GetTexture(); // Get texture from highest level mip

            texture->UploadData(deviceContext, PipelineStage::Pixel, registerOffset++);
        }

        if (m_depthBuffer != nullptr)
        {
            m_depthBuffer->GetTexture()->UploadData(deviceContext, PipelineStage::Pixel, registerOffset);
        }
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