#pragma once
#include <Utils\Loader\Data.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <memory>
#include <map>

// Data specific to imported textures. e.g. a list of 
// materials that require it and the texture type
// e.g. Diffuse, Specular, Opacity, etc...
struct ImportedTextureData
{
    std::vector<uint32_t> MaterialIDs;
    aiTextureType Type;
};

class Importer
{
public:
    Importer();

    // Returns an empty string if successful or the error if unsuccessful.
    std::string ReadFile(std::string filePath);

    Utils::Loader::SceneData GetSceneData() const { return m_sceneData; }
private:
    void LoadScene(const aiScene* importedScene);
    void LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene);
    void LoadMeshData(Utils::Loader::SceneNodeData& sceneNodeData, const aiMesh* mesh);
    void LoadMaterialData(Utils::Loader::MaterialData& materialData, const aiMaterial* mesh);
    void LoadTextures();

    void AddImportedTexture(const aiString& texturePath, const aiTextureType& type, const uint32_t& materialID);
private:
    std::unique_ptr<Assimp::Importer> m_aiImporter;
    // As we process the materials we save all the textures that need to be fetched
    // into the map which is then processed later to load all the textures from disk 
    // and associate each texture with the required materials.
    std::map<std::string, ImportedTextureData> m_importedTextures;
    Utils::Loader::SceneData m_sceneData;
    uint32_t m_currentNodeID = 0;
};

