#include "TextureManager.h"
#include <cctype>
#include <codecvt>
#include <DirectXTex.h>
#include <iostream>
#include <locale>
#include <Shlwapi.h>
#include <Utils\DirectxHelpers\EngineHelpers.h>

TextureManager::TextureManager()
{
    // Initialize the COM library.
    Utils::DirectXHelpers::ThrowIfFailed(CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED));
}

void TextureManager::AddImportedTexture(const aiString& texturePath, const aiTextureType& type, const uint32_t& materialID)
{
    if (texturePath.length > 0)
    {
        std::wstring path = GetPathAsWideString(std::string(texturePath.C_Str()));
        auto it = m_importedTextures.find(path);
        if (it != m_importedTextures.end())
        {
            it->second.MaterialIDs.push_back(materialID);
            // We presume that two texture are not used for different data.
            // e.g. Diffuse texture used as specular
        }
        else
        {
            ImportedTextureData textureData;
            textureData.MaterialIDs.push_back(materialID);
            textureData.Type = type;

            m_importedTextures[path] = textureData;
        }
    }
}

void TextureManager::ProcessTextures()
{
    for (auto texture : m_importedTextures)
    {
        auto texturePath = texture.first;
        auto textureData = texture.second;

        LoadTextureFromFile(texturePath, textureData.Type);
        // Need to search local directory for texture

        // When done need to load texture

        // Potentially decompress texture 

        // Potentially need to compress texture into desired format.

        // Create TextureData to add to scene.

        // Update materials to point towards texture ID
    }
}

void TextureManager::LoadTextureFromFile(const std::wstring& path, const aiTextureType& type)
{
    // Determine texture type
    auto realFilePath = FindFile(path);
    if (!realFilePath.empty())
    {
        auto extensionPosition = path.find_first_of('.', 0);
        std::wstring extension = path.substr(extensionPosition + 1);

        // Transform the file extension to upper case
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char c) { return std::toupper(c); });

        DirectX::ScratchImage image;
        DirectX::TexMetadata metadata;
        auto success = GetImageFromFile(realFilePath, image, metadata);
        if (success)
        {
            // Now depending on the type of texture e.g. Diffuse, Specular, Normal, etc..
            // We may need to do some additional processing.
            
            switch (type)
            {
                case aiTextureType_DIFFUSE:
                {
                    // Convert to premultiplied alpha
                    DWORD flags = 0;
                    DirectX::PremultiplyAlpha(image.GetImages(), image.GetImageCount(), metadata, flags, image);
                    // Generate MipMaps
                    DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), metadata, flags, 0, image);
                    // Compress BC3
                    break;
                }
                case aiTextureType_SPECULAR:
                {
                    // Generate MipMaps
                    // Compress BC3
                    break;
                }
                case aiTextureType_NORMALS:
                {
                    // Generate MipMaps
                    // Compress Bc3
                    break;
                }
            }
        }
    }
    else
    {
        // WARNINNG
        // COULD NOT FIND FILE
        std::wcout << "Could not find Texture file with path: " << path << std::endl;
    }
}

std::wstring TextureManager::FindFile(const std::wstring& filePath)
{
    // Check to see if path is a direct path on user hard drive e.g. c:\, E:\ */
    auto colonPos = filePath.find(':');
    bool isDirectPath = colonPos != filePath.npos;
    if (isDirectPath)
    {
        if (PathFileExists(filePath.c_str()))
        {
            return filePath;
        }
    }

    auto slashPos = filePath.find(L"/\\");
    bool inFolder = slashPos != filePath.npos;
    // If not try to load file from current directory e.g. currentDirectory + filePath
    std::wstring localPath = m_currentFileDirectory + L"\\" + filePath;
    if (PathFileExists(localPath.c_str()))
    {
        return localPath;
    }

    // If cant find file. Retrieve file name from path e.g. Textures\Diffuse.png = Diffuse.png
    // and then search current directory for any files with given file name and extension even searching
    // subfolders.
    std::wstring fileName = filePath;
    if (inFolder)
    {
        auto lastSlashPos = fileName.find_last_of(L"/\\");
        fileName = fileName.substr(lastSlashPos + 1);
        localPath = m_currentFileDirectory + L"\\" + fileName;
        if (PathFileExists(localPath.c_str()))
        {
            return localPath;
        }
    }


    // If still cant find file. Give up. Post warning.

    return std::wstring();
}

std::wstring TextureManager::GetPathAsWideString(const std::string& path)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(path);
}

bool TextureManager::GetImageFromFile(const std::wstring& path, DirectX::ScratchImage& image, DirectX::TexMetadata& metadata)
{
    auto extensionPosition = path.find_first_of('.', 0);
    std::wstring extension = path.substr(extensionPosition + 1);

    // Transform the file extension to upper case
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return std::toupper(c); });

    if (extension == L"DDS")
    {
        DWORD flags = 0;
        auto hr = DirectX::GetMetadataFromDDSFile(path.c_str(), flags, metadata);
        if (hr != S_OK)
        {
            std::wcout << "Failed to get metadata for file: " << path << std::endl;
            return false;
        }

        hr = DirectX::LoadFromDDSFile(path.c_str(), flags, &metadata, image);
        if (hr != S_OK)
        {
            std::wcout << "Failed to load image file: " << path << std::endl;
            return false;
        }
    }
    else if (extension == L"TGA")
    {
        DWORD flags = 0;
        auto hr = DirectX::GetMetadataFromTGAFile(path.c_str(), metadata);
        if (hr != S_OK)
        {
            std::wcout << "Failed to get metadata for file: " << path << std::endl;
            return false;
        }

        hr = DirectX::LoadFromTGAFile(path.c_str(), &metadata, image);
        if (hr != S_OK)
        {
            std::wcout << "Failed to load image file: " << path << std::endl;
            return false;
        }
    }
    else // Presume WIC supported format
    {
        DWORD flags = 0;
        auto hr = DirectX::GetMetadataFromWICFile(path.c_str(), flags, metadata);
        if (hr != S_OK)
        {
            std::wcout << "Failed to get metadata for file: " << path << std::endl;
            return false;
        }

        hr = DirectX::LoadFromWICFile(path.c_str(), flags, &metadata, image);
        if (hr != S_OK)
        {
            std::wcout << "Failed to load image file: " << path << std::endl;
            return false;
        }
    }

    return true;
}