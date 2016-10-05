#include "Importer.h"
#define NOMINMAX

#include <Utils\Math\Math.h>
#include <Utils\DirectxHelpers\EngineHelpers.h>
#include <DirectXTex.h>

#include <algorithm>
using namespace Utils::Loader;

float ConvertShininessToSmoothness(float shininess)
{
    return Utils::Maths::Clamp((log(shininess) - log(2)) / (10.0 * log(2)), 0.0, 1.0);
}

Importer::Importer()
{
    m_aiImporter = std::make_unique<Assimp::Importer>();
    m_textureManager = std::make_unique<TextureManager>();
}

std::string Importer::ReadFile(const std::string& directory, std::string& fileNameAndExtension)
{
    m_textureManager->SetAssetPath(directory);
    auto filePath = directory.empty() ? fileNameAndExtension : directory + "\\" + fileNameAndExtension;
    auto loaderFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals;
    auto importedScene = m_aiImporter->ReadFile(filePath, loaderFlags);
    std::string error;
    if (importedScene != nullptr)
    {
        LoadScene(importedScene);
    }
    else
    {
        error = m_aiImporter->GetErrorString();
    }
    // Now that we have finished import we free the scene so the loader can be used again.
    m_aiImporter->FreeScene();
    return error;
}

void Importer::LoadScene(const aiScene* importedScene)
{
    auto rootNodeID = m_currentNodeID++;
    LoadNode(importedScene->mRootNode, rootNodeID, importedScene);
    LoadTextures();
}

void Importer::LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene)
{
    //TODO: Need to add bounds component.
    // Add a new node to the scene to represent the imported node
    SceneNodeData sceneNode;
    sceneNode.NodeID = m_currentNodeID;
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

    // After loading all data increment the ID of the current node.
    m_currentNodeID++;

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
        auto aiPosition = mesh->mVertices[i];
        auto enginePosition = Utils::Maths::Vector3( aiPosition.x, aiPosition.y, aiPosition.z );
        // Update the SceneNode axis aligned bounding box to contain the new position
        sceneNode.Bounds.AddPosition(enginePosition);
        sceneNode.Positions.push_back(enginePosition);

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
    aiColor3D diffuse;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

    aiColor3D specular;
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);

    aiColor3D emissive;
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);

    float opacity;
    material->Get(AI_MATKEY_OPACITY, opacity);

    float reflectivity;
    material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);

    float shininess;
    material->Get(AI_MATKEY_SHININESS, shininess);

    aiString diffuseTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexture);

    aiString specularTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), specularTexture);

    aiString emissiveTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), emissiveTexture);

    aiString normalTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), normalTexture);

    aiString shininessTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_SHININESS, 0), shininessTexture);

    aiString opacityTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_OPACITY, 0), opacityTexture);

    aiString ambientOcclusionTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_LIGHTMAP, 0), ambientOcclusionTexture);

    // TODO: some formats may save shininess as PBR roughness so
    // we may need to check what range shininess fits in.
    float smoothness = ConvertShininessToSmoothness(shininess);

    materialData.ID = m_currentNodeID;
    materialData.DiffuseColor = { diffuse.r, diffuse.g, diffuse.b, opacity };
    materialData.SpecularColor = { specular.r, specular.g, specular.b, smoothness };
    materialData.EmissiveColor = { emissive.r, emissive.g, emissive.b };

    // TODO: Tidy this up a bit. A lot of redundant calls.
    materialData.DiffuseTextureID = m_textureManager->AddImportedTexture(diffuseTexture, aiTextureType_DIFFUSE);
    materialData.SpecularTextureID = m_textureManager->AddImportedTexture(specularTexture, aiTextureType_SPECULAR);
    materialData.EmissiveTextureID = m_textureManager->AddImportedTexture(emissiveTexture, aiTextureType_EMISSIVE);
    materialData.NormalTextureID = m_textureManager->AddImportedTexture(normalTexture, aiTextureType_NORMALS);
    materialData.SmoothnessTextureID = m_textureManager->AddImportedTexture(shininessTexture, aiTextureType_SHININESS);
    materialData.OpacityTextureID = m_textureManager->AddImportedTexture(opacityTexture, aiTextureType_OPACITY);
    materialData.AOTextureID = m_textureManager->AddImportedTexture(ambientOcclusionTexture, aiTextureType_LIGHTMAP);
}

void Importer::LoadTextures()
{
    m_textureManager->ProcessTextures();
    m_sceneData.Textures = m_textureManager->GetLoadedTextures();
}
