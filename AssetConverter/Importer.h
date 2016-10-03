#pragma once
#include "TextureManager.h"
#include <Utils\Loader\Data.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <memory>
#include <map>

class Importer
{
public:
    Importer();

    // Returns an empty string if successful or the error if unsuccessful.
    std::string ReadFile(const std::string& directory, std::string& fileNameAndExtension);

    Utils::Loader::SceneData GetSceneData() const { return m_sceneData; }
private:
    void LoadScene(const aiScene* importedScene);
    void LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene);
    void LoadMeshData(Utils::Loader::SceneNodeData& sceneNodeData, const aiMesh* mesh);
    void LoadMaterialData(Utils::Loader::MaterialData& materialData, const aiMaterial* mesh);
    void LoadTextures();

private:
    std::unique_ptr<Assimp::Importer> m_aiImporter;
    std::unique_ptr<TextureManager> m_textureManager;
    // As we process the materials we save all the textures that need to be fetched
    // into the map which is then processed later to load all the textures from disk 
    // and associate each texture with the required materials.
    Utils::Loader::SceneData m_sceneData;
    uint32_t m_currentNodeID = 0;
    std::string m_currentFileDirectory;
};

