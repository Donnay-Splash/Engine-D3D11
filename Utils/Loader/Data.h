#pragma once
#include <stdint.h>
#include <Utils\Math\Math.h>
#include <vector>

namespace Utils
{
    namespace Loader
    {
        static const uint32_t kVersionNumber = 1;
        static const uint32_t SceneNodeDataID = 0x01;
        static const uint32_t MaterialDataID = 0x02;
        static const uint32_t TextureDataID = 0x04;

        static const uint32_t kInvalidID = ~0;

        struct SceneNodeData
        {
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
            // Will be necessary if we want to avoid material duplication
            uint32_t ID = kInvalidID;
            Utils::Maths::Color DiffuseColor;
            Utils::Maths::Color SpecularColor;
            Utils::Maths::Color EmissiveColor;
            // Will need to add bunch of texture IDs

            uint32_t DiffuseTextureID = kInvalidID;
            uint32_t SpecularTextureID = kInvalidID;
            uint32_t EmissiveTextureID = kInvalidID;
            uint32_t NormalTextureID = kInvalidID;
            uint32_t ShininessTextureID = kInvalidID;
            uint32_t OpacityTextureID = kInvalidID;
            uint32_t AOTextureID = kInvalidID;
        };

        struct TextureData
        {
            uint32_t ID = kInvalidID;
            uint32_t Width = 0;
            uint32_t Height = 0;
            uint32_t BitsPerPixel = 0;
            std::vector<uint8_t> data;
        };

        struct SceneData
        {
            std::vector<SceneNodeData> SceneNodes;
            std::vector<MaterialData> Materials;
            std::vector<TextureData> Textures;
        };

    }
}