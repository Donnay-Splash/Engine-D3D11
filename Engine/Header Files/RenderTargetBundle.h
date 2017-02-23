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

        RenderTargetBundle(ID3D11Device* device, uint32_t width, uint32_t height, uint32_t arraySize = 1);

        void CreateRenderTarget(std::wstring name, DXGI_FORMAT format);

        RenderTarget::Ptr GetRenderTarget(std::wstring name);

        DepthBuffer::Ptr GetDepthBuffer() const { return m_depthBuffer; }

        void Finalise();

        void Clear(ID3D11DeviceContext* deviceContext);

        // Uploads all render targets as SRV's. User must ensure that the 
        // render targets are not bound for output.
        void SetShaderResources(ID3D11DeviceContext* deviceContext);

        // Returns a pointer to the array or render targets
        ID3D11RenderTargetView * const* GetRenderTargetViews();

        uint32_t RenderTargetCount() const { return m_count; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }

    private:
        struct BundleElement
        {
            std::wstring Name;
            RenderTarget::Ptr RenderTarget;
        };

    private:
        std::vector<BundleElement> m_renderTargets;
        DepthBuffer::Ptr m_depthBuffer;
        Sampler::Ptr m_bundleSampler;
        std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_renderTargetViews;

        // Number of render targets stored in this bundle
        uint32_t m_count = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_arraySize = 0;

        bool m_finalised = false;

        // Pointer to the device this bundle was created with.
        ID3D11Device* m_device;

    };
}// End namespace Engine