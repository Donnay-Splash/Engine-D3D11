#include "TextureManager.h"
#include <iostream>
#include <cctype>

TextureManager::TextureManager()
{

}

void TextureManager::AddImportedTexture(const aiString& texturePath, const aiTextureType& type, const uint32_t& materialID)
{
    if (texturePath.length > 0)
    {
        std::string path(texturePath.C_Str());
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

        // Need to search local directory for texture

        // When done need to load texture

        // Potentially decompress texture 

        // Potentially need to compress texture into desired format.

        // Create TextureData to add to scene.

        // Update materials to point towards texture ID
    }
}

void TextureManager::LoadTextureFromFile(const std::string& path, const aiTextureType& type)
{
    // Determine texture type
    auto extensionPosition = path.find_first_of('.', 0);
    std::string extension = path.substr(extensionPosition + 1);
    auto fileNamePosition = path.find_last_of("/\\", 0);
    std::string fileName = path.substr(fileNamePosition + 1, extensionPosition);

    /*std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return std::toupper(c); });*/

    if (extension == "DDS")
    {

    }
    else if (extension == "TGA")
    {

    }
    else // Presume WIC supported format
    {

    }

}

void TextureManager::FindFile(const std::string& currentDirectory, const std::string& fileName, const std::string& extension)
{
    // Check to see if any slashes in file path
    // 
    //// If it doesn't exist see if the given directory exists
    //DWORD fileType = GetFileAttributesA(outputDirectory.c_str());
    //if (fileType == INVALID_FILE_ATTRIBUTES)
    //{
    //    std::cout << outputDirectory << " is not a valid directory.";
    //    return false;
    //}
    //else if ((fileType & FILE_ATTRIBUTE_DIRECTORY) == 0)
    //{
    //    std::cout << outputDirectory << " is not a valid directory.";
    //    return false;
    //}

    // If it doesn't search the current file directory for a file with the same name and extension.
}