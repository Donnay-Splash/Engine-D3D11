#include "Importer.h"

Importer::Importer(std::string filePath)
{
    m_aiImporter = std::make_unique<Assimp::Importer>();
    auto loaderFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
    auto importedScene = m_aiImporter->ReadFile(filePath, loaderFlags);
    LoadScene(importedScene);
     // Now that we have finished import we free the scene so the loader can be used again.
    m_aiImporter->FreeScene();
}

void Importer::LoadScene(const aiScene* importedScene)
{
    auto rootNodeID = m_currentNodeID++;
    LoadNode(importedScene->mRootNode, rootNodeID, importedScene);
}

void Importer::LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene)
{
    //TODO: Need to add bounds component.
    // Add a new node to the scene to represent the imported node
    SceneNodeData sceneNode;
    sceneNode.NodeID = m_currentNodeID++;
    sceneNode.ParentID = parentNodeID;
    if (importedNode->mNumMeshes > 0)
    {
        for (unsigned int i = 0; i < importedNode->mNumMeshes; i++)
        {
            // Load the mesh information.
            auto meshIndex = importedNode->mMeshes[i];
            auto mesh = importedScene->mMeshes[meshIndex];
            sceneNode.LoadMeshData(mesh);
    
            // Load the material for the mesh
            auto materialIndex = mesh->mMaterialIndex;
            auto material = importedScene->mMaterials[materialIndex];
            MaterialData materialData;
            materialData.LoadMaterialData(material);
            m_sceneData.Materials.push_back(materialData);
        }
    }
    
    // Set the transformation on the SceneNode
    auto t = importedNode->mTransformation;
    Utils::Maths::Matrix engineTransform = { t.a1, t.a2, t.a3, t.a4,
                                            t.b1, t.b2, t.b3, t.b4,
                                            t.c1, t.c2, t.c3, t.c4,
                                            t.d1, t.d2, t.d3, t.d4 };
    sceneNode.Transform = engineTransform;
    
    // Finally add the scene node to the scene data
    m_sceneData.SceneNodes.push_back(sceneNode);

    // Load the sceneNodes children
    for (unsigned int i = 0; i < importedNode->mNumChildren; i++)
    {
        LoadNode(importedNode->mChildren[i], sceneNode.NodeID, importedScene);
    }
}
//
//void Loader::LoadNode(const aiNode* importedNode, const SceneNode::Ptr parentNode, const aiScene* importedScene)
//{
//    // TODO: Need to add bounds component.
//    // Add a new node to the scene to represent the imported node
//    auto sceneNode = m_scene->AddNode(parentNode);
//    if (importedNode->mNumMeshes > 0)
//    {
//        // If there are meshes attached to the node then we need to create a MeshInstance component
//        // to render these meshes.
//        auto meshInstance = sceneNode->AddComponent<MeshInstance>(m_d3dClass->GetDevice());
//        for (unsigned int i = 0; i < importedNode->mNumMeshes; i++)
//        {
//            // Load the mesh information.
//            auto meshIndex = importedNode->mMeshes[i];
//            auto mesh = importedScene->mMeshes[meshIndex];
//            auto engineMesh = std::make_shared<Mesh>();
//            engineMesh->Load(mesh, m_d3dClass->GetDevice());
//            meshInstance->SetMesh(engineMesh);
//
//            // Load the material for the mesh
//            auto materialIndex = mesh->mMaterialIndex;
//            auto material = importedScene->mMaterials[materialIndex];
//            auto engineMaterial = std::make_shared<Material>(m_d3dClass->GetDevice(), m_shaderPipeline);
//            engineMaterial->Load(material);
//            meshInstance->SetMaterial(engineMaterial);
//        }
//    }
//
//    // Set the transformation on the SceneNode
//    auto t = importedNode->mTransformation;
//    Utils::Maths::Matrix engineTransform = { t.a1, t.a2, t.a3, t.a4,
//        t.b1, t.b2, t.b3, t.b4,
//        t.c1, t.c2, t.c3, t.c4,
//        t.d1, t.d2, t.d3, t.d4 };
//    sceneNode->SetTransform(engineTransform);
//
//    // Load the sceneNodes children
//    for (unsigned int i = 0; i < importedNode->mNumChildren; i++)
//    {
//        LoadNode(importedNode->mChildren[i], sceneNode, importedScene);
//    }
//}