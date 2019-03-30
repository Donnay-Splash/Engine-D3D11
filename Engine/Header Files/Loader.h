#pragma once

#include "d3dclass.h"
#include <Scene\Scene.h>
#include <Resources\ShaderPipeline.h>
#include <Resources\Texture.h>
#include <Resources\Material.h>
#include <Utils\Loader\MikeLoader.h>
#include <map>

namespace Engine
{
    // TODO: Create mode advanced loader options
    class Loader
    {
    public:
        using Ptr = std::shared_ptr<Loader>;
        Loader(Scene::Ptr scene, ShaderPipeline::Ptr shaderPipeline);
        ~Loader();

        void LoadFile(const std::string& filePath);
        void LoadFile(const uint8_t* data, uint64_t byteCount);
    private:
        void LoadScene(const Utils::Loader::SceneData& importedScene);
        void LoadSceneNodes(const std::vector<Utils::Loader::SceneNodeData>& importedSceneNodes);
        void LoadMaterials(const std::vector<Utils::Loader::MaterialData>& importedMaterials);
        void LoadTextures(const std::vector<Utils::Loader::TextureData>& importedTextures);
        void LoadNode(const Utils::Loader::SceneNodeData& importedNode);

    private:
        Scene::Ptr m_scene;
        ShaderPipeline::Ptr m_shaderPipeline;
        std::map<uint32_t, SceneNode::Ptr> m_nodeMap;
        std::map<uint32_t, Material::Ptr> m_materialMap;
        std::map<uint32_t, Texture::Ptr> m_textureMap;
    };
}