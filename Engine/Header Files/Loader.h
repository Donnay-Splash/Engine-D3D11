#pragma once

#include "d3dclass.h"
#include <Scene\Scene.h>
#include <Resources\ShaderPipeline.h>
#include <Utils\Loader\MikeLoader.h>
#include <map>

// TODO: Create mode advanced loader options
class Loader
{
public:
    using Ptr = std::shared_ptr<Loader>;
    Loader(D3DClass::Ptr d3dClass, Scene::Ptr scene, ShaderPipeline::Ptr shaderPipeline);
    ~Loader();

    void LoadFile(const std::string& filePath);
private:
    void LoadScene(const Utils::Loader::SceneData& importedScene);
    void LoadNode(const Utils::Loader::SceneNodeData& importedNode);

private:
    D3DClass::Ptr m_d3dClass;
    Scene::Ptr m_scene;
    ShaderPipeline::Ptr m_shaderPipeline;
    std::map<uint32_t, SceneNode::Ptr> m_nodeMap;
};