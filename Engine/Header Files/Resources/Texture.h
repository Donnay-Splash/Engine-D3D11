#pragma once
#include <Utils\Loader\Data.h>
namespace TextureCreationFlags
{
    static const uint32_t BindRenderTarget = 0x0001;
    static const uint32_t BindShaderResource = 0x0002;
    static const uint32_t BindDepthStencil = 0x0004;
}

class Texture
{
public:

    using Ptr = std::shared_ptr<Texture>;
    // Create a texture from set of parameters
    Texture(void* data, uint32_t width, uint32_t height, uint32_t flags, DXGI_FORMAT textureFormat, ID3D11Device* device);
    // Create texture from D3D11 resource.
    Texture(ID3D11Texture2D* texture, uint32_t flags, ID3D11Device* device);
    // Create a texture from a file
    Texture(ID3D11Device* device, const wchar_t* filename);
    // Create a texture from imported texture data
    Texture(ID3D11Device* device, const Utils::Loader::TextureData& importedTextureData);
    Texture(const Texture&) = delete;

    void UploadData(ID3D11DeviceContext* deviceContext, uint32_t pipelineStage, uint32_t textureRegister);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture() { return m_texture; }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() { return m_srv; }

    uint32_t GetHeight() const { return m_height; }
    uint32_t GetWidth() const { return m_width; }

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

    uint32_t m_height;
    uint32_t m_width;
    // format

};
