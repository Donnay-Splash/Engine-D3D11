#pragma once
#include <assimp\material.h>
#include <map>
#include <memory>
#include <vector>
#include <DirectXTex.h>
#include <Utils\Loader\Data.h>

// Data specific to imported textures. e.g. a list of 
// materials that require it and the texture type
// e.g. Diffuse, Specular, Opacity, etc...
struct ImportedTextureData
{
    uint32_t ID;
    aiTextureType Type;
};


// Due to the large amount of texture processing that is required in the importer
// the TextureManager is needed to separate that code to make it easier to manage.
class TextureManager
{
public:
    using Ptr = std::shared_ptr<TextureManager>;
    TextureManager();

    // Add a texture that is required by a material. A request may have already been put in to load
    // this particular texture, if so then we will update the loaded texture information to include the
    // referenced material.
    // returns the ID of the texture so the material can find it on load
    uint32_t AddImportedTexture(const aiString& texturePath, const aiTextureType& type);

    void SetAssetPath(const std::string& currentFileDirectory) { m_currentFileDirectory = GetPathAsWideString(currentFileDirectory); }
    
    void ProcessTextures();

    std::vector<Utils::Loader::TextureData> GetLoadedTextures() const { return m_loadedTextures; }

private:
    void LoadTextureFromFile(const std::wstring& path, const ImportedTextureData& textureInfo);
    std::wstring FindFile(const std::wstring& filePath);
    std::wstring GetPathAsWideString(const std::string& path);
    bool GetImageFromFile(const std::wstring& path, DirectX::ScratchImage& image);

private:
    std::map<std::wstring, ImportedTextureData> m_importedTextures;
    std::vector<Utils::Loader::TextureData> m_loadedTextures;
    std::wstring m_currentFileDirectory;

    uint32_t m_currentID = 0;
};