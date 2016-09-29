#pragma once
#include <Utils\Loader\Data.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <memory>

class Importer
{
public:
    Importer(std::string filePath);

    Utils::Loader::SceneData GetSceneData() const { return m_sceneData; }
private:
    void LoadScene(const aiScene* importedScene);
    void LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene);
    void LoadMeshData(Utils::Loader::SceneNodeData& sceneNodeData, const aiMesh* mesh);
    void LoadMaterialData(Utils::Loader::MaterialData& materialData, const aiMaterial* mesh);
private:
    std::unique_ptr<Assimp::Importer> m_aiImporter;
    Utils::Loader::SceneData m_sceneData;
    uint32_t m_currentNodeID = 0;
};

