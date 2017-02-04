#pragma once
#include <Utils\Loader\Data.h>
#include <Engine\Build\pch.h>

namespace Engine
{
    namespace TextureCreationFlags
    {
        static const uint32_t BindRenderTarget = 0x0001;
        static const uint32_t BindShaderResource = 0x0002;
        static const uint32_t BindDepthStencil = 0x0004;
        static const uint32_t SRGB = 0x0008;
        static const uint32_t Multisampled = 0x0010;
    }

    // Texture class. Currently only supports 2D textures and arrays.
    class Texture
    {
    public:

        using Ptr = std::shared_ptr<Texture>;

        static Texture::Ptr CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t flags, DXGI_FORMAT format, ID3D11Device* device);
        static Texture::Ptr CreateTextureArray(void* data, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t flags, DXGI_FORMAT format, ID3D11Device* device);
        // static Texture::Ptr CreateCubeMapTexture(); // Not yet implemented
        static Texture::Ptr CreateTextureFromResource(ID3D11Texture2D* texture, uint32_t flags, ID3D11Device* device);
        static Texture::Ptr CreateTextureFromFile(std::wstring filename, ID3D11Device* device);
        static Texture::Ptr CreateImportedTexture(const Utils::Loader::TextureData& importedData, ID3D11Device* device);

        // Creates a new texture with exactly the same layout as the given texture
        static Texture::Ptr CreateIdenticalTexture(Texture::Ptr const texture, ID3D11Device* device);

        void UploadData(ID3D11DeviceContext* deviceContext, uint32_t pipelineStage, uint32_t textureRegister);

        DXGI_FORMAT GetFormat() const { return m_format; }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture() { return m_texture; }
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() { return m_srv; }

        uint32_t GetHeight() const { return m_height; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetArraySize() const { return m_arraySize; }

    protected:
        // Due to the many possible ways of creating a texture
        // the textures can only be created via more explicit
        // static methods. Thus the constructors are kept protected.

        // Create a texture from set of parameters
        Texture(void* data, D3D11_TEXTURE2D_DESC desc, ID3D11Device* device);
        // Create texture from D3D11 resource.
        Texture(ID3D11Texture2D* texture, uint32_t flags, ID3D11Device* device);
        // Create a texture from a file
        Texture(ID3D11Device* device, const wchar_t* filename);
        // Create a texture from imported texture data
        Texture(ID3D11Device* device, const Utils::Loader::TextureData& importedTextureData);
        Texture(const Texture&) = delete;

    private:
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
        DXGI_FORMAT m_format;

        uint32_t m_height;
        uint32_t m_width;
        uint32_t m_arraySize;
    };
}
