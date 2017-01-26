#pragma once

#include <Resources\RenderTarget.h>
#include <Resources\DepthBuffer.h>

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

        RenderTargetBundle(uint32_t width, uint32_t height, ID3D11Device* device);

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

        std::map<std::wstring, RenderTarget::Ptr> m_renderTargetMap;
        DepthBuffer::Ptr m_depthBuffer;
        std::array<ID3D11RenderTargetView* , D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_renderTargetViews;

        // Number of render targets stored in this bundle
        uint32_t m_count = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;

        bool m_finalised = false;

        // Pointer to the device this bundle was created with.
        ID3D11Device* m_device;

    };
}// End namespace Engine