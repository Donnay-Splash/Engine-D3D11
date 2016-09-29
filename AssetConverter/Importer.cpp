#include "Importer.h"
#include <Utils\Math\Math.h>
#include <Utils\DirectxHelpers\EngineHelpers.h>
using namespace Utils::Loader;

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
            LoadMeshData(sceneNode, mesh);
    
            // Load the material for the mesh
            auto materialIndex = mesh->mMaterialIndex;
            auto material = importedScene->mMaterials[materialIndex];
            MaterialData materialData;
            LoadMaterialData(materialData, material);
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

void Importer::LoadMeshData(Utils::Loader::SceneNodeData& sceneNode, const aiMesh* mesh)
{
    // TODO: Need to report any conversion failures to console.
    // at a minimum we expect the mesh to atleast contain vertex positions
    EngineAssert(mesh->HasPositions());
    //// A mesh must also have faces 
    EngineAssert(mesh->HasFaces());
    //// We only support triangles
    EngineAssert(mesh->mFaces[0].mNumIndices == 3);
    sceneNode.HasPositions = true;
    sceneNode.HasNormals = mesh->HasNormals();
    sceneNode.HasUVs = mesh->HasTextureCoords(0);
    sceneNode.HasTangents = mesh->HasTangentsAndBitangents();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        auto position = mesh->mVertices[i];
        sceneNode.Positions.push_back({ position.x, position.y, position.z });

        if (sceneNode.HasNormals)
        {
            auto normal = mesh->mNormals[i];
            sceneNode.Normals.push_back({ normal.x, normal.y, normal.z });
        }

        if (sceneNode.HasUVs)
        {
            auto textureCoord = mesh->mTextureCoords[0][i];
            sceneNode.UVs.push_back({ textureCoord.x, textureCoord.y });
        }

        if (sceneNode.HasTangents)
        {
            auto tangent = mesh->mTangents[i];
            auto bitangent = mesh->mBitangents[i];

            sceneNode.Tangents.push_back({ tangent.x, tangent.y, tangent.z });
            sceneNode.Bitangents.push_back({ bitangent.x, bitangent.y, bitangent.z });
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        auto face = mesh->mFaces[i];
        sceneNode.Indices.push_back(face.mIndices[0]);
        sceneNode.Indices.push_back(face.mIndices[1]);
        sceneNode.Indices.push_back(face.mIndices[2]);
    }

    sceneNode.IndexCount = static_cast<uint32_t>(sceneNode.Indices.size());
    sceneNode.VertexCount = static_cast<uint32_t>(sceneNode.Positions.size());
}

void Importer::LoadMaterialData(MaterialData& materialData, const aiMaterial* material)
{

}
