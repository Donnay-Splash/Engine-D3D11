#include "pch.h"
#include "RenderTarget.h"

RenderTarget::RenderTarget(uint32_t width, uint32_t height, uint32_t creationFlags, DXGI_FORMAT format, ID3D11Device* device)
{
    creationFlags |= TextureCreationFlags::BindRenderTarget;
    m_texture = std::make_shared<Texture>(nullptr, width, height, creationFlags, format, device);
    Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), NULL, m_renderTargetView.GetAddressOf()));
}

RenderTarget::RenderTarget(ID3D11Texture2D* texture, uint32_t creationFlags, ID3D11Device* device)
{
    m_texture = std::make_shared<Texture>(texture, creationFlags, device);
    Utils::DirectXHelpers::ThrowIfFailed(device->CreateRenderTargetView(m_texture->GetTexture().Get(), NULL, m_renderTargetView.GetAddressOf()));
}

RenderTarget::~RenderTarget()
{

}