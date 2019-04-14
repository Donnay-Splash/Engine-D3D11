#include "pch.h"
#include <Resources\TextureMipView.h>
#include <Resources\ConstantBuffer.h>

namespace Engine
{
    TextureMipView::TextureMipView(Texture::Ptr texture, uint32_t mipLevels /* = 0*/) :
        m_mipLevels(mipLevels), m_currentMip(0), m_texture(texture)
    {
        EngineAssert(m_texture != nullptr);
        auto d3dTex = m_texture->GetResource();
        D3D12_RESOURCE_DESC texDesc = d3dTex->GetDesc();

        uint32_t textureMipCount = texDesc.MipLevels;
        EngineAssert(m_mipLevels <= textureMipCount);
        if (m_mipLevels == 0) m_mipLevels = textureMipCount;

        m_mipSRVs.resize(m_mipLevels);
        for (uint32_t i = 0; i < m_mipLevels; i++)
        {
            // Create an SRV for each mip level.
            // Each SRV will be able to access the single mip and no more.
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

            // For now we will deal with flat textures not texture arrays
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Format = texDesc.Format;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = i;

			IMPLEMENT_FOR_DX12(device->CreateShaderResourceView(d3dTex.Get(), &srvDesc, m_mipSRVs[i].GetAddressOf());)
        }
    }

    void TextureMipView::UploadData(ID3D11DeviceContext* deviceContext, uint32_t pipelineStage, uint32_t textureRegister)
    {
        if (pipelineStage & PipelineStage::Pixel)
        {
            deviceContext->PSSetShaderResources(textureRegister, 1, m_mipSRVs[m_currentMip].GetAddressOf());
        }

        if (pipelineStage & PipelineStage::Vertex)
        {
            deviceContext->VSSetShaderResources(textureRegister, 1, m_mipSRVs[m_currentMip].GetAddressOf());
        }
    }
}