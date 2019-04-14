#pragma once
#include <Utils\Loader\Data.h>
#include <Engine\Build\pch.h>
#include "ShaderResource.h"

namespace Engine
{
    // Forward declaration
    class D3DClass;

    namespace TextureCreationFlags
    {
        static const uint32_t BindRenderTarget = 0x0001;
        static const uint32_t BindShaderResource = 0x0002;
        static const uint32_t BindDepthStencil = 0x0004;
        static const uint32_t SRGB = 0x0008;
        static const uint32_t Multisampled = 0x0010;
        static const uint32_t GenerateMIPs = 0x0020;
    }

    // Texture class. Currently only supports 2D textures and arrays.
    class Texture : public ShaderResource, public GPUResource
    {
    public:
        using Ptr = std::shared_ptr<Texture>;

        static Texture::Ptr CreateTexture(void* data, uint32_t width, uint32_t height, uint32_t flags, DXGI_FORMAT format);
        static Texture::Ptr CreateTextureArray(void* data, uint32_t width, uint32_t height, uint32_t arraySize, uint32_t flags, DXGI_FORMAT format);
        // static Texture::Ptr CreateCubeMapTexture(); // Not yet implemented
        static Texture::Ptr CreateTextureFromResource(ID3D12Resource* texture, uint32_t flags);
        static Texture::Ptr CreateTextureFromFile(std::wstring filename);
        static Texture::Ptr CreateImportedTexture(const Utils::Loader::TextureData& importedData);
        static Texture::Ptr CreateTextureFromMemory(const uint8_t* data, uint64_t byteCount, bool generateMips = false);

        // Creates a new texture with exactly the same layout as the given texture
        static Texture::Ptr CreateIdenticalTexture(Texture::Ptr const texture);

        void UploadData(ID3D12GraphicsCommandList* commandList, uint32_t textureRegister);

        DXGI_FORMAT GetFormat() const { return m_format; }

        uint32_t GetHeight() const { return m_height; }
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetArraySize() const { return m_arraySize; }
        uint32_t GetMipLevels() const { return m_mipLevels; }
        bool IsCubeMap() const { return m_isCube; }

    protected:
        // Due to the many possible ways of creating a texture
        // the textures can only be created via more explicit
        // static methods. Thus the constructors are kept protected.

        // Create a texture from set of parameters
        Texture(void* data, D3D12_RESOURCE_DESC desc);
        // Create texture from D3D11 resource.
        Texture(ID3D12Resource* texture, uint32_t flags);
        // Create a texture from a file
        Texture(const wchar_t* filename);
        // Create a texture from memory
        Texture(const uint8_t* data, uint64_t byteCount, bool generateMips = false);
        Texture(const Texture&) = delete;

    private:
        DXGI_FORMAT m_format;

        uint32_t m_height;
        uint32_t m_width;
        uint32_t m_arraySize;
        uint32_t m_mipLevels;
        bool m_isCube = false;
    };
}
