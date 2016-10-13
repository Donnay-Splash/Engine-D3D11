#include "stdafx.h"
#include "Importer.h"
#include <Utils\Math\Math.h>
#include <Utils\DirectxHelpers\EngineHelpers.h>
#include <DirectXTex.h>
#include <iostream>
#include <algorithm>
using namespace Utils::Loader;

// To get data prepared for d3d
uint32_t postProcessSteps = aiProcess_CalcTangentSpace | // calculate tangents and bitangents if possible
                            aiProcess_JoinIdenticalVertices | // join identical vertices/ optimize indexing
                            aiProcess_ValidateDataStructure | // perform a full validation of the loader's output
                            aiProcess_ImproveCacheLocality | // improve the cache locality of the output vertices
                            aiProcess_RemoveRedundantMaterials | // remove redundant materials
                            aiProcess_FindDegenerates | // remove degenerated polygons from the import
                            aiProcess_FindInvalidData | // detect invalid model data, such as invalid normal vectors
                            aiProcess_GenUVCoords | // convert spherical, cylindrical, box and planar mapping to proper UVs
                            aiProcess_TransformUVCoords | // preprocess UV transformations (scaling, translation ...)
                            aiProcess_OptimizeMeshes | // join small meshes, if possible
                            aiProcess_Triangulate | // Converts all polygons to triangles
                            aiProcess_GenSmoothNormals |
                            aiProcess_ConvertToLeftHanded; // Generates smooth normals if none exist 

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
    auto importedScene = m_aiImporter->ReadFile(filePath, postProcessSteps);
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
    LoadTextures(importedScene->mTextures, importedScene->mNumTextures);
}

void Importer::LoadNode(const aiNode* importedNode, uint32_t parentNodeID, const aiScene* importedScene)
{
    // Add a new node to the scene to represent the imported node
    std::vector<SceneNodeData> sceneNodes;

    // To handle that case of multiple nodes needing to be created
    // we only want the child nodes to be parented to one so we save that
    // ID here
    auto firstNodeID = m_currentNodeID;

    // Get the transform for the node
    auto t = importedNode->mTransformation;
    Utils::Maths::Matrix engineTransform = { t.a1, t.a2, t.a3, t.a4,
        t.b1, t.b2, t.b3, t.b4,
        t.c1, t.c2, t.c3, t.c4,
        t.d1, t.d2, t.d3, t.d4 };

    // If there is more that one mesh for this node we want to separate it
    // into multiple nodes
    if (importedNode->mNumMeshes > 0)
    {
        for (unsigned int i = 0; i < importedNode->mNumMeshes; i++)
        {
            SceneNodeData sceneNode;
            sceneNode.NodeID = m_currentNodeID++;
            sceneNode.ParentID = parentNodeID;

            // Load the mesh information.
            auto meshIndex = importedNode->mMeshes[i];
            auto mesh = importedScene->mMeshes[meshIndex];
            LoadMeshData(sceneNode, mesh);
            sceneNodes.push_back(sceneNode);
    
            // Load the material for the mesh
            auto materialIndex = mesh->mMaterialIndex;
            auto material = importedScene->mMaterials[materialIndex];
            MaterialData materialData;
            LoadMaterialData(materialData, material, sceneNode.NodeID);
            m_sceneData.Materials.push_back(materialData);
        }
    }
    else
    {
        SceneNodeData sceneNode;
        sceneNode.NodeID = m_currentNodeID++;
        sceneNode.ParentID = parentNodeID;
        sceneNodes.push_back(sceneNode);
    }
    
    for (auto sceneNode : sceneNodes)
    {
        sceneNode.Transform = engineTransform;
        m_sceneData.SceneNodes.push_back(sceneNode);
    }
    // Load the sceneNodes children
    for (unsigned int i = 0; i < importedNode->mNumChildren; i++)
    {
        LoadNode(importedNode->mChildren[i], firstNodeID, importedScene);
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
    bool hasUV2 = mesh->HasTextureCoords(1);
    bool hasUV3 = mesh->HasTextureCoords(2);
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

void Importer::LoadMaterialData(MaterialData& materialData, const aiMaterial* material, uint32_t materialID)
{
    aiString materialName;
    material->Get(AI_MATKEY_NAME, materialName);
    std::cout << "Loading material with name: " << materialName.C_Str() << std::endl;

    aiColor3D diffuse;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

    aiColor3D specular;
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);

    aiColor3D emissive;
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);

    float opacity = 1.0f;
    material->Get(AI_MATKEY_OPACITY, opacity);

    float reflectivity = 0.0f;
    material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);

    float shininess = 0.5f;
    material->Get(AI_MATKEY_SHININESS, shininess);

    aiString diffuseTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), diffuseTexture);

    aiString specularTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), specularTexture);

    aiString emissiveTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), emissiveTexture);

    aiString ambientTexture;
    material->Get(AI_MATKEY_TEXTURE(aiTextureType_AMBIENT, 0), ambientTexture);

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
    float smoothness = shininess;
    if (shininess > 1)
    {
        float smoothness = ConvertShininessToSmoothness(shininess);
    }

    materialData.ID = materialID;
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

void Importer::LoadTextures(aiTexture** embeddedTextures, uint32_t embeddedTextureCount)
{
    m_textureManager->ProcessTextures(embeddedTextures, embeddedTextureCount);
    m_sceneData.Textures = m_textureManager->GetLoadedTextures();
}

