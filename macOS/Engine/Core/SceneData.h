#ifndef ENGINE_CORE_SCENE_DATA_H
#define ENGINE_CORE_SCENE_DATA_H

#include "Math.h"

#include <cstdint>
#include <string>
#include <vector>

namespace engine {
namespace core {

static const std::uint32_t InvalidIndex = UINT32_MAX;

struct MeshData
{
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector3> tangents;
    std::vector<Vector3> bitangents;
    std::vector<Vector2> texCoords;
    std::vector<std::uint32_t> indices;

    Bounds bounds() const;
    bool validate(std::string &error) const;
};

struct MaterialData
{
    Vector4 diffuseColor;
    Vector4 specularColor;
    Vector4 emissiveColor;
    float opacity;
    float smoothness;

    MaterialData();
};

struct SceneNodeData
{
    std::uint32_t parentIndex;
    std::uint32_t meshIndex;
    std::uint32_t materialIndex;
    Matrix4 localTransform;

    SceneNodeData();
};

struct SceneData
{
    std::vector<SceneNodeData> nodes;
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;

    bool validate(std::string &error) const;
};

} // namespace core
} // namespace engine

#endif
