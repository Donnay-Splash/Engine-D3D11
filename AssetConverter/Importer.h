#pragma once
#include "Data.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <memory>

class Importer
{
public:
    Importer(std::string filePath);

    SceneData GetSceneData() const { return m_sceneData; }
private:
    std::unique_ptr<Assimp::Importer> m_aiImporter;
    void LoadScene(const aiScene* importedScene);
    void LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene);
private:
    SceneData m_sceneData;
    uint32_t m_currentNodeID = 0;
};