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

void Loader::LoadScene(const SceneData& importedScene)
{
    auto rootNode = m_scene->GetRootNode();
    for (const auto& sceneNode : importedScene.SceneNodes)
    {
        LoadNode(sceneNode);
    }

    m_nodeMap.clear();
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

        // Material will be added later when it is loaded from the scene.
        // For now add temp empty material
        Material::Ptr material = std::make_shared<Material>(m_d3dClass->GetDevice(), m_shaderPipeline);
        meshInstance->SetMaterial(material);
    }
}