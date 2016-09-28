#pragma once
#include <stdint.h>
#include <Math\Math.h>
#include <vector>

struct SceneNodeData
{
    SceneNodeData();
    // Required data. Every instance of a SceneNode will supply this data.
    uint32_t NodeID;
    uint32_t ParentID;
    Utils::Maths::Matrix Transform;
    uint32_t VertexCount;
    uint32_t IndexCount;
    bool HasPositions; // if vertex count is more than 0 this should be true
    bool HasNormals;
    bool HasTangents;
    bool HasUVs;

    // Now we store the Mesh Data.
    std::vector<Utils::Maths::Vector3> Positions;
    std::vector<Utils::Maths::Vector3> Normals;
    std::vector<Utils::Maths::Vector3> Tangents;
    std::vector<Utils::Maths::Vector3> Bitangents;
    std::vector<Utils::Maths::Vector2> UVs;
};

struct MaterialData
{

};

struct TextureData
{

};