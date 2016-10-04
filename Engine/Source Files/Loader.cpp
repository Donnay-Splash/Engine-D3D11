#include "pch.h"
#include "Loader.h"
#include <Resources\Material.h>
#include <Resources\Mesh.h>
#include <Scene\Components\MeshInstance.h>

using namespace Utils::Loader;

Loader::Loader(D3DClass::Ptr d3dClass, Scene::Ptr scene, ShaderPipeline::Ptr shaderPipeline) :
    m_d3dClass(d3dClass), m_scene(scene), m_shaderPipeline(shaderPipeline)
{
}

Loader::~Loader()
{

}

void Loader::LoadFile(const std::string& filePath)
{
    // Eventually we can add additional flags to calculate normals and tangents
    // cache optimisations, etc...
    MikeLoader importer;
    auto importedScene = importer.LoadFile(filePath);
    LoadScene(importedScene);
}

void Loader::LoadFile(const uint8_t* data, uint64_t byteCount)
{
    MikeLoader importer;
    auto importedScene = importer.LoadFile(data, byteCount);
    LoadScene(importedScene);
}

void Loader::LoadScene(const SceneData& importedScene)
{
    LoadTextures(importedScene.Textures);
    LoadMaterials(importedScene.Materials);
    LoadSceneNodes(importedScene.SceneNodes);

    m_textureMap.clear();
    m_materialMap.clear();
    m_nodeMap.clear();
}


void Loader::LoadSceneNodes(const std::vector<Utils::Loader::SceneNodeData>& importedSceneNodes)
{
    for (auto importedNode : importedSceneNodes)
    {
        LoadNode(importedNode);
    }
}

void Loader::LoadNode(const SceneNodeData& importedNode)
{
    // TODO: Need to add bounds component.
    // Add a new node to the scene to represent the imported node
    SceneNode::Ptr sceneNode;
    if (importedNode.ParentID == 0)
    {
        sceneNode = m_scene->AddNode();
    }
    else
    {
        bool parentExists = (m_nodeMap.find(importedNode.ParentID) != m_nodeMap.end());
        EngineAssert(parentExists);
        auto parentNode = m_nodeMap[importedNode.ParentID];
        sceneNode = m_scene->AddNode(parentNode);
    }

    // Now that we have created our new node add it to the map so
    // other nodes can parent to it.
    m_nodeMap[importedNode.NodeID] = sceneNode;
    sceneNode->SetTransform(importedNode.Transform);

    // If the node has vertices assigned then load the mesh data into a mesh object
    // and attach a MeshInstance to the node
    if (importedNode.VertexCount > 0)
    {
        Mesh::Ptr mesh = std::make_shared<Mesh>();
        mesh->Load(importedNode, m_d3dClass->GetDevice());

        MeshInstance::Ptr meshInstance = sceneNode->AddComponent<MeshInstance>(m_d3dClass->GetDevice());
        meshInstance->SetMesh(mesh);

        // Retrieve the material for this node. All materials and SceneNodes
        // should share the same ID
        EngineAssert(m_materialMap.find(importedNode.NodeID) != m_materialMap.end());
        Material::Ptr material = m_materialMap[importedNode.NodeID];
        meshInstance->SetMaterial(material);
    }
}

void Loader::LoadMaterials(const std::vector<Utils::Loader::MaterialData>& importedMaterials)
{
    for (auto importedMaterial : importedMaterials)
    {

    }
}

void Loader::LoadTextures(const std::vector<Utils::Loader::TextureData>& importedTextures)
{
    for (auto importedTexture : importedTextures)
    {
        // Need to ensure texture loaded correctly
        auto engineTexture = std::make_shared<Texture>(m_d3dClass->GetDevice(), importedTexture);
        // This value must not exist in the map already
        EngineAssert(m_textureMap.find(importedTexture.ID) == m_textureMap.end());
        m_textureMap[importedTexture.ID] = engineTexture;
    }
}