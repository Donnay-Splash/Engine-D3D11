#include "SceneData.h"

namespace engine {
namespace core {
namespace {

bool sameCountOrEmpty(std::size_t count, std::size_t positionsCount)
{
    return count == 0 || count == positionsCount;
}

} // namespace

Bounds MeshData::bounds() const
{
    Bounds result;
    for (std::vector<Vector3>::const_iterator position = positions.begin();
         position != positions.end();
         ++position) {
        result.include(*position);
    }
    return result;
}

bool MeshData::validate(std::string &error) const
{
    error.clear();
    if (!sameCountOrEmpty(normals.size(), positions.size())
        || !sameCountOrEmpty(tangents.size(), positions.size())
        || !sameCountOrEmpty(bitangents.size(), positions.size())
        || !sameCountOrEmpty(texCoords.size(), positions.size())) {
        error = "Mesh vertex attributes must be empty or match the position count.";
        return false;
    }
    if (positions.empty() && !indices.empty()) {
        error = "A mesh with indices must contain positions.";
        return false;
    }
    for (std::vector<std::uint32_t>::const_iterator index = indices.begin();
         index != indices.end();
         ++index) {
        if (*index >= positions.size()) {
            error = "Mesh index is outside the position array.";
            return false;
        }
    }
    return true;
}

MaterialData::MaterialData()
    : diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
      specularColor(0.0f, 0.0f, 0.0f, 1.0f),
      emissiveColor(0.0f, 0.0f, 0.0f, 1.0f),
      opacity(1.0f),
      smoothness(0.0f)
{
}

SceneNodeData::SceneNodeData()
    : parentIndex(InvalidIndex),
      meshIndex(InvalidIndex),
      materialIndex(InvalidIndex),
      localTransform()
{
}

bool SceneData::validate(std::string &error) const
{
    error.clear();
    for (std::size_t index = 0; index < meshes.size(); ++index) {
        if (!meshes[index].validate(error)) {
            return false;
        }
    }
    for (std::size_t index = 0; index < nodes.size(); ++index) {
        const SceneNodeData &node = nodes[index];
        if (node.parentIndex != InvalidIndex && node.parentIndex >= nodes.size()) {
            error = "Scene node parent index is outside the node array.";
            return false;
        }
        if (node.parentIndex == index) {
            error = "A scene node cannot be its own parent.";
            return false;
        }
        if (node.meshIndex != InvalidIndex && node.meshIndex >= meshes.size()) {
            error = "Scene node mesh index is outside the mesh array.";
            return false;
        }
        if (node.materialIndex != InvalidIndex && node.materialIndex >= materials.size()) {
            error = "Scene node material index is outside the material array.";
            return false;
        }
    }
    return true;
}

} // namespace core
} // namespace engine
