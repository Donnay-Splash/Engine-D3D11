#include "pch.h"
#include "DepthBuffer.h"

DepthBuffer::DepthBuffer(uint32_t width, uint32_t height, uint32_t creationFlags, ID3D11Device* device)
{
    creationFlags |= TextureCreationFlags::BindRenderTarget;
    m_texture = std::make_shared<Texture>(nullptr, width, height, creationFlags, DXGI_FORMAT_D24_UNORM_S8_UINT, device);
    Utils::ThrowIfFailed(device->CreateDepthStencilView(m_texture->GetTexture().Get(), NULL, m_dsv.GetAddressOf()));
}