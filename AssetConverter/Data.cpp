#include "Data.h"

void SceneNodeData::LoadMeshData(const aiMesh* mesh)
{
    // TODO: Need to report any conversion failures to console.
    // at a minimum we expect the mesh to atleast contain vertex positions
    //EngineAssert(mesh->HasPositions());
    //// A mesh must also have faces 
    //EngineAssert(mesh->HasFaces());
    //// We only support triangles
    //EngineAssert(mesh->mFaces[0].mNumIndices == 3);
    HasPositions = true;
    HasNormals = mesh->HasNormals();
    HasUVs = mesh->HasTextureCoords(0);
    HasTangents = mesh->HasTangentsAndBitangents();
    
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        auto position = mesh->mVertices[i];
        Positions.push_back({ position.x, position.y, position.z });
    
        if (HasNormals)
        {
            auto normal = mesh->mNormals[i];
            Normals.push_back({ normal.x, normal.y, normal.z });
        }
    
        if (HasUVs)
        {
            auto textureCoord = mesh->mTextureCoords[0][i];
            UVs.push_back({ textureCoord.x, textureCoord.y });
        }
    
        if (HasTangents)
        {
            auto tangent = mesh->mTangents[i];
            auto bitangent = mesh->mBitangents[i];
    
            Tangents.push_back({ tangent.x, tangent.y, tangent.z });
            Bitangents.push_back({ bitangent.x, bitangent.y, bitangent.z });
        }
    }
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        auto face = mesh->mFaces[i];
        Indices.push_back(face.mIndices[0]);
        Indices.push_back(face.mIndices[1]);
        Indices.push_back(face.mIndices[2]);
    }

    IndexCount = static_cast<uint32_t>(Indices.size());
    VertexCount = static_cast<uint32_t>(Positions.size());

}