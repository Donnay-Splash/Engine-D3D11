#pragma once

#include <assimp\mesh.h>
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Mesh
{
public:
    using PositionType = Utils::Maths::Vector3;
    using NormalType = Utils::Maths::Vector3;
    using TangentType = Utils::Maths::Vector3;
    using UVType = Utils::Maths::Vector2;
    using IndexType = uint32_t;
    using PositionContainer = std::vector<PositionType>;
    using NormalContainer = std::vector<NormalType>;
    using TangentContainer = std::vector<TangentType>;
    using UVContainer = std::vector<UVType>;
    using IndexContainer = std::vector<IndexType>;


public:
    using Ptr = std::shared_ptr<Mesh>;
    Mesh();
    Mesh(const Mesh&) = delete;
    ~Mesh();

    void Load(aiMesh* mesh, ID3D11Device* device);

    void Render(ID3D11DeviceContext*);
    void FinaliseMesh(ID3D11Device* device);
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indexData.size()); };
    uint32_t GetVertexCount() const { return static_cast<uint32_t>(m_vertexCount); }

    void SetPositions(const PositionContainer& positions);
    void SetNormals(const NormalContainer& normals);
    void SetTangents(const TangentContainer& tangents, const TangentContainer& bitangents);
    void SetUVs(const UVContainer& uvCoords);
    void SetIndices(const IndexContainer& indices);

    // TODO: Mesh should be a create once object.
    // Need a finalize mesh function that can do additional final processing.
    bool HasPositions() const { return !m_positionData.empty(); }
    bool HasNormals() const { return !m_normalData.empty(); }
    bool HasUVs() const { return !m_uvData.empty(); }

    PositionContainer GetPositions() const { return m_positionData; };
    NormalContainer GetNormals() const { return m_normalData; };
    UVContainer GetUVs() const { return m_uvData; };
    IndexContainer GetIndices() const{ return m_indexData; }

    
private:
    void RenderBuffers(ID3D11DeviceContext*);
    
    std::shared_ptr<Texture> m_Texture;
    
protected:	
    
    // Raw Data
    PositionContainer m_positionData;
    NormalContainer m_normalData;
    TangentContainer m_tangentData;
    TangentContainer m_bitangentData;
    UVContainer m_uvData;
    IndexContainer m_indexData;

    // Buffers
    VertexBuffer::Ptr m_positionBuffer;
    VertexBuffer::Ptr m_normalBuffer;
    VertexBuffer::Ptr m_uvBuffer;
    // VertexBuffer::Ptr UV1;
    // VertexBuffer::Ptr tangents;
    // VertexBuffer::Ptr bitangents;
    // VertexBuffer::Ptr colors;
    IndexBuffer::Ptr m_indexBuffer;

    size_t m_vertexCount;
    bool m_meshFinalised = false;
};
