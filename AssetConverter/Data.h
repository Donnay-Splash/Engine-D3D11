#pragma once
#include <stdint.h>
#include <Math\Math.h>
#include <vector>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

static const uint32_t kVersionNumber = 1;
static const uint32_t SceneNodeDataID = 0x01;
static const uint32_t MaterialDataID = 0x02;
static const uint32_t TextureDataID = 0x04;

struct SceneNodeData
{
    void LoadMeshData(const aiMesh* mesh);
    // Required data. Every instance of a SceneNode will supply this data.
    uint32_t NodeID = 0;
    uint32_t ParentID = 0;
    Utils::Maths::Matrix Transform;
    uint32_t VertexCount = 0;
    uint32_t IndexCount = 0;
    bool HasPositions = false; // if vertex count is more than 0 this should be true
    bool HasNormals = false;
    bool HasTangents = false;
    bool HasUVs = false;

    // Now we store the Mesh Data.
    std::vector<Utils::Maths::Vector3> Positions;
    std::vector<Utils::Maths::Vector3> Normals;
    std::vector<Utils::Maths::Vector3> Tangents;
    std::vector<Utils::Maths::Vector3> Bitangents;
    std::vector<Utils::Maths::Vector2> UVs;
    std::vector<uint32_t> Indices;
};

struct MaterialData
{
    void LoadMaterialData(const aiMaterial* material) {}
};

struct TextureData
{
    void LoadTextureData(const aiTexture* texture) {}
};

struct SceneData
{
    std::vector<SceneNodeData> SceneNodes;
    std::vector<MaterialData> Materials;
    std::vector<TextureData> Textures;
};