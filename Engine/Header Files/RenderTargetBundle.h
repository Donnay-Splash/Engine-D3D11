#pragma once

#include <Resources\RenderTarget.h>
#include <Resources\DepthBuffer.h>
#include <Resources\Sampler.h>
#include <unordered_map>

namespace Engine
{
    /*
        Stores an array of render targets for submitting to the device.
        To ensure that the render targets meet the requirements the
        RenderTargetBundle class handles creation of all resources.
    */
    class RenderTargetBundle
    {
    public:
        using Ptr = std::shared_ptr<RenderTargetBundle>;

        /*
            Params:
            device - pointer to the D3D11Device
            width - width of the render targets in pixels
            height - height of the render targets in pixels
            arraySize - When creating texture arrays and render target arrays this sets the size
            mipLevels - Used when we want to manually render to different mip levels of the render target.
                        A value less than one will perform as normal. A value larger than one will generate
                        render targets to draw to each of the different levels of the mip chain. Which mip is
                        being drawn to can be changed by calling SetTargetMip()
        */
        RenderTargetBundle(uint32_t width, uint32_t height, uint32_t arraySize = 1, uint32_t mipLevels = 1, bool depthEnabled = true);

        void CreateRenderTarget(std::wstring name, DXGI_FORMAT format, const Utils::Maths::Color& clearColor = {});

        RenderTarget::Ptr GetRenderTarget(std::wstring name);
        RenderTarget::Ptr GetRenderTarget(uint32_t index);

        DepthBuffer::Ptr GetDepthBuffer() const { return m_depthBuffer; }

        void Finalise();

        void Clear(ID3D11DeviceContext* deviceContext);

        // Uploads all render targets as SRV's. User must ensure that the 
        // render targets are not bound for output.
        void SetShaderResources(ID3D11DeviceContext* deviceContext, uint32_t registerOffset = 0);

        // Returns a pointer to the array or render targets
        ID3D11RenderTargetView * const* GetRenderTargetViews();

        uint32_t RenderTargetCount() const { return m_count; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
        uint32_t GetMipLevels() const { return m_mipLevels; }

        void SetTargetMip(uint32_t targetMip);

    private:
        struct BundleElement
        {
            std::wstring Name;
            std::vector<RenderTarget::Ptr> RenderTargets; // Render targets for all mip levels
            Utils::Maths::Color ClearColour = {0.0f, 0.0f, 0.0f, 1.0f};
        };

    private:
        std::vector<BundleElement> m_renderTargets;
        DepthBuffer::Ptr m_depthBuffer;
        Sampler::Ptr m_bundleSampler;
        // Stores a vector of render target arrays for rendering to each mip level of the texture resource
        std::vector<std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT>> m_renderTargetViews;

        // Number of render targets stored in this bundle
        uint32_t m_count = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_arraySize = 0;
        uint32_t m_mipLevels = 0;
        uint32_t m_targetMip = 0;

        bool m_finalised = false;
    };
}// End namespace Engine