#include "pch.h"
#include <Resources\Mesh.h>

using namespace Utils::Loader;

namespace Engine
{
    Mesh::Mesh()
    {
        m_vertexCount = 0;
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::Load(const SceneNodeData& mesh)
    {
        // at a minimum we expect the mesh to atleast contain vertex positions
        EngineAssert(mesh.HasPositions);
        // A mesh must also have faces 
        EngineAssert(mesh.IndexCount > 0);

        // Now finalise the mesh.
        SetPositions(mesh.Positions);
        SetIndices(mesh.Indices);
        if (mesh.HasNormals)
        {
            SetNormals(mesh.Normals);
        }
        if (mesh.HasUVs)
        {
            SetUVs(mesh.UVs);
        }
        if (mesh.HasTangents)
        {
            SetTangents(mesh.Tangents, mesh.Bitangents);
        }

        FinaliseMesh();
    }

    void Mesh::Render(ID3D12GraphicsCommandList* commandList)
    {
        // Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
        RenderBuffers(commandList);

        return;
    }

    void Mesh::RenderBuffers(ID3D12GraphicsCommandList* commandList)
    {
        UINT index = 0;
        // The mesh must have been finalised before we can begin rendering it.
        EngineAssert(m_meshFinalised);
        if (HasPositions())
        {
            // If the mesh has been finalised the position buffer should not be empty.
            EngineAssert(m_positionBuffer != nullptr);
            m_positionBuffer->UploadData(commandList, index);
            index++;
        }
        if (HasNormals())
        {
            // If the mesh has been finalised the normal buffer should not be empty.
            EngineAssert(m_normalBuffer != nullptr);
            m_normalBuffer->UploadData(commandList, index);
            index++;
        }
        if (HasUVs())
        {
            // If the mesh has been finalised the UV buffer should not be empty.
            EngineAssert(m_uvBuffer != nullptr);
            m_uvBuffer->UploadData(commandList, index);
            index++;
        }
        if (HasTangents())
        {
            EngineAssert(m_tangentBuffer != nullptr);
            EngineAssert(m_bitangentBuffer != nullptr);
            m_tangentBuffer->UploadData(commandList, index);
            index++;
            m_bitangentBuffer->UploadData(commandList, index);
            index++;
        }

        m_indexBuffer->UploadData(commandList);

        // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->DrawInstanced(GetIndexCount(), 1, 0, 0);

        return;
    }

    void Mesh::FinaliseMesh()
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
            m_positionBuffer = std::make_shared<VertexBuffer>(m_positionData.data(), static_cast<uint32_t>(m_positionData.size()), sizeof(PositionType));
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
            m_normalBuffer = std::make_shared<VertexBuffer>(m_normalData.data(), static_cast<uint32_t>(m_normalData.size()), sizeof(NormalType));
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
            m_uvBuffer = std::make_shared<VertexBuffer>(m_uvData.data(), static_cast<uint32_t>(m_uvData.size()), sizeof(UVType));
            m_vertexCount = m_uvData.size();
        }
        if (HasTangents())
        {
            // Ensure we have the same number of tangents and bitangents
            EngineAssert(m_tangentData.size() == m_bitangentData.size());
            if (m_vertexCount != 0)
            {
                // We should have a tangent and bitangent for each vertex
                EngineAssert(m_tangentData.size() == m_vertexCount);
                EngineAssert(m_bitangentData.size() == m_vertexCount);
            }
            // Tangent and bitangent buffers shouldn't have been created yet.
            EngineAssert(m_tangentBuffer == nullptr && m_bitangentBuffer == nullptr);
            // Generate tangent and bitangent vertex buffers
            m_tangentBuffer = std::make_shared<VertexBuffer>(m_tangentData.data(), static_cast<uint32_t>(m_tangentData.size()), sizeof(TangentType));
            m_bitangentBuffer = std::make_shared<VertexBuffer>(m_bitangentData.data(), static_cast<uint32_t>(m_bitangentData.size()), sizeof(TangentType));
            m_vertexCount = m_tangentData.size();
        }

        m_indexBuffer = std::make_shared<IndexBuffer>(m_indexData.data(), static_cast<uint32_t>(m_indexData.size()));
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
        // Must be the same number of tangents and bitangents
        EngineAssert(tangents.size() == bitangents.size());

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
}
