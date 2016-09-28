#pragma once

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "d3dclass.h"
#include "Scene.h"
#include "ShaderPipeline.h"

// TODO: Create mode advanced loader options
class Loader
{
public:
    using Ptr = std::shared_ptr<Loader>;
    Loader(D3DClass::Ptr d3dClass, Scene::Ptr scene, ShaderPipeline::Ptr shaderPipeline);
    ~Loader();

    void LoadFile(const std::string& filePath);
private:
    void LoadScene(const aiScene* importedScene);
    void LoadNode(const aiNode* importedNode, const SceneNode::Ptr parentNode, const aiScene* importedScene);

private:
    std::unique_ptr<Assimp::Importer> m_importer;
    D3DClass::Ptr m_d3dClass;
    Scene::Ptr m_scene;
    ShaderPipeline::Ptr m_shaderPipeline;
};