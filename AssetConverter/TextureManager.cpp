#include "TextureManager.h"
#include <cctype>
#include <codecvt>
#include <iostream>
#include <locale>
#include <Shlwapi.h>
#include <Utils\DirectxHelpers\EngineHelpers.h>

TextureManager::TextureManager()
{
    // Initialize the COM library.
    Utils::DirectXHelpers::ThrowIfFailed(CoInitializeEx(NULL, COINIT::COINIT_MULTITHREADED));
}

uint32_t TextureManager::AddImportedTexture(const aiString& texturePath, const aiTextureType& type)
{
    if (texturePath.length > 0)
    {
        std::wstring path = GetPathAsWideString(std::string(texturePath.C_Str()));
        auto it = m_importedTextures.find(path);
        if (it != m_importedTextures.end())
        {
            return it->second.ID;
        }
        else
        {
            ImportedTextureData textureData;
            textureData.ID = m_currentID++;
            textureData.Type = type;

            m_importedTextures[path] = textureData;
            return textureData.ID;
        }
    }
    return Utils::Loader::kInvalidID;
}

void TextureManager::ProcessTextures()
{
    for (auto texture : m_importedTextures)
    {
        auto texturePath = texture.first;
        auto textureInfo = texture.second;

        LoadTextureFromFile(texturePath, textureInfo);
        // Need to search local directory for texture

        // When done need to load texture

        // Potentially decompress texture 

        // Potentially need to compress texture into desired format.

        // Create TextureData to add to scene.

        // Update materials to point towards texture ID
    }
}

void TextureManager::LoadTextureFromFile(const std::wstring& path, const ImportedTextureData& textureInfo)
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

        // The original image loaded from disk without any processing
        DirectX::ScratchImage rawImage;
        // The final compressed image with all processing that we will save to disk.
        DirectX::ScratchImage compressedImage;
        auto success = GetImageFromFile(realFilePath, rawImage);
        if (success)
        {
            // Now depending on the type of texture e.g. Diffuse, Specular, Normal, etc..
            // We may need to do some additional processing.
            // For now we are skipping Opacity, Shininess and AO textures. Till I find resources
            // to test with. 
            switch (textureInfo.Type)
            {
                case aiTextureType_DIFFUSE:
                {
                    // Convert to premultiplied alpha
                    DWORD flags = 0;
                    DirectX::ScratchImage preMultipliedImage;
                    DirectX::PremultiplyAlpha(rawImage.GetImages(), rawImage.GetImageCount(), rawImage.GetMetadata(), flags, preMultipliedImage);
                    // Generate MipMaps
                    DirectX::ScratchImage mipMapImage;
                    DirectX::GenerateMipMaps(preMultipliedImage.GetImages(), preMultipliedImage.GetImageCount(), preMultipliedImage.GetMetadata(), flags, 0, mipMapImage);
                    // Compress BC3
                    DirectX::Compress(mipMapImage.GetImages(), mipMapImage.GetImageCount(), mipMapImage.GetMetadata(), DXGI_FORMAT_BC3_UNORM_SRGB, flags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
                    break;
                }
                case aiTextureType_SPECULAR:
                case aiTextureType_EMISSIVE:
                {
                    DWORD flags = 0;
                    // Generate MipMaps
                    DirectX::ScratchImage mipMapImage;
                    DirectX::GenerateMipMaps(rawImage.GetImages(), rawImage.GetImageCount(), rawImage.GetMetadata(), flags, 0, mipMapImage);
                    // Compress BC3
                    DirectX::Compress(mipMapImage.GetImages(), mipMapImage.GetImageCount(), mipMapImage.GetMetadata(), DXGI_FORMAT_BC3_UNORM_SRGB, flags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
                    break;
                }
                case aiTextureType_NORMALS:
                {
                    DWORD flags = 0;
                    // Generate MipMaps
                    DirectX::ScratchImage mipMapImage;
                    DirectX::GenerateMipMaps(rawImage.GetImages(), rawImage.GetImageCount(), rawImage.GetMetadata(), flags, 0, mipMapImage);
                    // Compress Bc3
                    DWORD compressFlags = DirectX::TEX_COMPRESS_UNIFORM;
                    DirectX::Compress(rawImage.GetImages(), rawImage.GetImageCount(), rawImage.GetMetadata(), DXGI_FORMAT_BC3_UNORM_SRGB, compressFlags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
                    break;
                }
            }

            // Now we have processed the data we will save it to DDS memory for storage in the .mike.
            DWORD flags = 0;
            DirectX::Blob fileData;
            DirectX::SaveToDDSMemory(compressedImage.GetImages(), compressedImage.GetImageCount(), compressedImage.GetMetadata(), flags, fileData);
            if (fileData.GetBufferSize() > 0)
            {
                Utils::Loader::TextureData textureData;
                textureData.ID = textureInfo.ID;
                textureData.dataSize = fileData.GetBufferSize();
                // Copy data to vector
                uint8_t* pixels = reinterpret_cast<uint8_t*>(fileData.GetBufferPointer());
                textureData.data = std::vector<uint8_t>(pixels, pixels + textureData.dataSize);
                m_loadedTextures.push_back(textureData);
            }
            else
            {
                std::wcout << "Failed to create DDS Texture for file: " << path << std::endl;
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
    std::wstring localPath = m_currentFileDirectory.empty() ? filePath : m_currentFileDirectory + L"\\" + filePath;
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
        localPath = m_currentFileDirectory.empty() ? fileName : m_currentFileDirectory + L"\\" + fileName;
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

bool TextureManager::GetImageFromFile(const std::wstring& path, DirectX::ScratchImage& image)
{
    auto extensionPosition = path.find_first_of('.', 0);
    std::wstring extension = path.substr(extensionPosition + 1);

    // Transform the file extension to upper case
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return std::toupper(c); });

    if (extension == L"DDS")
    {
        DWORD flags = 0;
        auto hr = DirectX::LoadFromDDSFile(path.c_str(), flags, nullptr, image);
        if (hr != S_OK)
        {
            std::wcout << "Failed to load image file: " << path << std::endl;
            return false;
        }
    }
    else if (extension == L"TGA")
    {
        DWORD flags = 0;
        auto hr = DirectX::LoadFromTGAFile(path.c_str(), nullptr, image);
        if (hr != S_OK)
        {
            std::wcout << "Failed to load image file: " << path << std::endl;
            return false;
        }
    }
    else // Presume WIC supported format
    {
        DWORD flags = 0;
        auto hr = DirectX::LoadFromWICFile(path.c_str(), flags, nullptr, image);
        if (hr != S_OK)
        {
            std::wcout << "Failed to load image file: " << path << std::endl;
            return false;
        }
    }

    return true;
}