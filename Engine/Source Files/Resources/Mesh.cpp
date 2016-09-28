#include "pch.h"
#include <Resources\Mesh.h>

Mesh::Mesh()
{
    m_vertexCount = 0;
}

Mesh::~Mesh()
{
}

void Mesh::Load(aiMesh* mesh, ID3D11Device* device)
{
    // at a minimum we expect the mesh to atleast contain vertex positions
    EngineAssert(mesh->HasPositions());
    // A mesh must also have faces 
    EngineAssert(mesh->HasFaces());
    // We only support triangles
    EngineAssert(mesh->mFaces[0].mNumIndices == 3);

    PositionContainer positions;
    NormalContainer normals;
    UVContainer textureCoordinates;
    TangentContainer tangents;
    TangentContainer bitangents;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        auto position = mesh->mVertices[i];
        positions.push_back({ position.x, position.y, position.z });

        if (mesh->HasNormals())
        {
            auto normal = mesh->mNormals[i];
            normals.push_back({ normal.x, normal.y, normal.z });
        }

        if (mesh->HasTextureCoords(0))
        {
            auto textureCoord = mesh->mTextureCoords[0][i];
            textureCoordinates.push_back({ textureCoord.x, textureCoord.y });
        }

        if (mesh->HasTangentsAndBitangents())
        {
            auto tangent = mesh->mTangents[i];
            auto bitangent = mesh->mBitangents[i];

            tangents.push_back({ tangent.x, tangent.y, tangent.z });
            bitangents.push_back({ bitangent.x, bitangent.y, bitangent.z });
        }
    }

    IndexContainer indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        auto face = mesh->mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    // Now finalise the mesh.
    SetPositions(positions);
    if (!normals.empty())
    {
        SetNormals(normals);
    }
    if (!textureCoordinates.empty())
    {
        SetUVs(textureCoordinates);
    }
    if (!tangents.empty() && !bitangents.empty())
    {
        SetTangents(tangents, bitangents);
    }

    SetIndices(indices);

    FinaliseMesh(device);
}

void Mesh::Render(ID3D11DeviceContext* deviceContext)
{
    // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
    RenderBuffers(deviceContext);

    return;
}

void Mesh::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
    UINT index = 0;
    // The mesh must have been finalised before we can begin rendering it.
    EngineAssert(m_meshFinalised);
    if (HasPositions())
    {
        // If the mesh has been finalised the position buffer should not be empty.
        EngineAssert(m_positionBuffer != nullptr);
        m_positionBuffer->UploadData(deviceContext, index, 0);
        index++;
    }
    if (HasNormals())
    {
        // If the mesh has been finalised the normal buffer should not be empty.
        EngineAssert(m_normalBuffer != nullptr);
        m_normalBuffer->UploadData(deviceContext, index, 0);
        index++;
    }
    if (HasUVs())
    {
        // If the mesh has been finalised the UV buffer should not be empty.
        EngineAssert(m_uvBuffer != nullptr);
        m_uvBuffer->UploadData(deviceContext, index, 0);
        index++;
    }

    m_indexBuffer->UploadData(deviceContext);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    deviceContext->DrawIndexed(GetIndexCount(), 0, 0);

    return;
}

void Mesh::FinaliseMesh(ID3D11Device* device)
{
    // Cannot finalise the same mesh twice. This function can only be called a single time per mesh.
    EngineAssert(!m_meshFinalised);
    EngineAssert(m_vertexCount == 0);
    // We cannot draw a mesh without any indices.
    EngineAssert(!m_indexData.empty());

    // Now we generate all of the vertex buffers for the position, normal and UV data
    if (HasPositions())
    {
        // The position buffer should not have been created yet.
        EngineAssert(m_positionBuffer == nullptr);
        // Generate position vertex buffer.
        m_positionBuffer = std::make_shared<VertexBuffer>(m_positionData.data(), static_cast<uint32_t>(m_positionData.size()), sizeof(PositionType), device);
        m_vertexCount = m_positionData.size();
    }
    if (HasNormals())
    {
        if (m_vertexCount != 0)
        {
            // There should be a normal per vertex of the mesh.
            EngineAssert(m_normalData.size() == m_vertexCount);
        }
        // The normal buffer should not have been created yet.
        EngineAssert(m_normalBuffer == nullptr);
        // Generate normal vertex buffer
        m_normalBuffer = std::make_shared<VertexBuffer>(m_normalData.data(), static_cast<uint32_t>(m_normalData.size()), sizeof(NormalType), device);
        m_vertexCount = m_normalData.size();
    }
    if (HasUVs())
    {
        if (m_vertexCount != 0)
        {
            // There should be a UV per vertex of the mesh.
            EngineAssert(m_uvData.size() == m_vertexCount);
        }
        // The UV buffer should not have been created yet.
        EngineAssert(m_uvBuffer == nullptr);
        // Generate UV vertex buffer
        m_uvBuffer = std::make_shared<VertexBuffer>(m_uvData.data(), static_cast<uint32_t>(m_uvData.size()), sizeof(UVType), device);
        m_vertexCount = m_uvData.size();
    }

    m_indexBuffer = std::make_shared<IndexBuffer>(m_indexData.data(), static_cast<uint32_t>(m_indexData.size()), device);
    m_meshFinalised = true;
}

void Mesh::SetPositions(const PositionContainer& positions)
{
    // Mesh has been finalised by this point so we cannot change the data.
    EngineAssert(!m_meshFinalised);
    // Data must not be set for a mesh twice.
    EngineAssert(m_positionData.empty());
    // Why are you passing an empty vector to the mesh?
    EngineAssert(!positions.empty());

    // Position data is stored. Later when FinaliseMesh() is called 
    // we will create the vertex buffers
    m_positionData = positions;
}

void Mesh::SetNormals(const NormalContainer& normals)
{
    // Mesh has been finalised by this point so we cannot change the data.
    EngineAssert(!m_meshFinalised);
    // Data must not be set for a mesh twice.
    EngineAssert(m_normalData.empty());
    // Why are you passing an empty vector to the mesh?
    EngineAssert(!normals.empty());

    // Normal data is stored. Later when FinaliseMesh() is called 
    // we will create the vertex buffers
    m_normalData = normals;
}

void Mesh::SetTangents(const TangentContainer& tangents, const TangentContainer& bitangents)
{
    // Mesh has been finalised by this point so we cannot change the data.
    EngineAssert(!m_meshFinalised);
    // Data must not be set for a mesh twice.
    EngineAssert(m_tangentData.empty());
    // Data must not be set for a mesh twice.
    EngineAssert(m_bitangentData.empty());
    // Why are you passing an empty vector to the mesh?
    EngineAssert(!tangents.empty());
    // Why are you passing an empty vector to the mesh?
    EngineAssert(!bitangents.empty());

    // Tangent data is stored. Later when FinaliseMesh() is called
    // we will create the vertex buffers
    m_tangentData = tangents;
    m_bitangentData = bitangents;
}

void Mesh::SetUVs(const UVContainer& uvs)
{
    // Mesh has been finalised by this point so we cannot change the data.
    EngineAssert(!m_meshFinalised);
    // Data must not be set for a mesh twice.
    EngineAssert(m_uvData.empty());
    // Why are you passing an empty vector to the mesh?
    EngineAssert(!uvs.empty());

    // UV data is stored. Later when FinaliseMesh() is called 
    // we will create the vertex buffers
    m_uvData = uvs;
}

void Mesh::SetIndices(const IndexContainer& indices)
{
    // Mesh has been finalised by this point so we cannot change the data.
    EngineAssert(!m_meshFinalised);
    // Data must not be set for a mesh twice.
    EngineAssert(m_indexData.empty());
    // Why are you passing an empty vector to the mesh?
    EngineAssert(!indices.empty());

    // index data is stored. Later when FinaliseMesh() is called 
    // we will create the vertex buffers
    m_indexData = indices;
}
