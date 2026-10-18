#include "Core/Math.h"
#include "Core/SceneData.h"

#include <cstddef>
#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace {

void check(bool condition, const char *message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool close(float actual, float expected)
{
    return std::fabs(actual - expected) < 0.0001f;
}

void rowVectorTransformsAndProjectionUseDocumentedConventions()
{
    using engine::core::Matrix4;
    using engine::core::Quaternion;
    using engine::core::Vector4;

    const Vector4 translated =
        Matrix4::translation({10.0f, -2.0f, 4.0f}).transform({1.0f, 2.0f, 3.0f, 1.0f});
    check(close(translated.x, 11.0f) && close(translated.y, 0.0f)
            && close(translated.z, 7.0f) && close(translated.w, 1.0f),
        "row-vector translation must use the final matrix row");

    const Matrix4 rotated = Matrix4::rotation(Quaternion::axisAngle(
        {0.0f, 0.0f, 1.0f}, 1.57079632679f));
    const Vector4 rotatedX = rotated.transform({1.0f, 0.0f, 0.0f, 1.0f});
    check(close(rotatedX.x, 0.0f) && close(rotatedX.y, 1.0f),
        "a positive quarter turn around Z must map +X to +Y");

    const Matrix4 projection = Matrix4::perspectiveLeftHanded(
        1.57079632679f, 1.0f, 1.0f, 11.0f);
    const Vector4 nearPoint = projection.transform({0.0f, 0.0f, 1.0f, 1.0f});
    const Vector4 farPoint = projection.transform({0.0f, 0.0f, 11.0f, 1.0f});
    check(close(nearPoint.z / nearPoint.w, 0.0f)
            && close(farPoint.z / farPoint.w, 1.0f),
        "left-handed perspective depth must map near to zero and far to one");

    const Matrix4 model = Matrix4::scaling({2.0f, 2.0f, 2.0f})
        * Matrix4::translation({10.0f, -2.0f, 4.0f});
    const Vector4 modelPoint = model.transform({1.0f, 2.0f, 3.0f, 1.0f});
    check(close(modelPoint.x, 12.0f) && close(modelPoint.y, 2.0f)
            && close(modelPoint.z, 10.0f),
        "row-vector matrix multiplication must apply scale before translation");

    check(std::is_standard_layout<Matrix4>::value
            && sizeof(Matrix4) == sizeof(float) * 16
            && offsetof(Matrix4, elements) == 0,
        "public matrices must have contiguous row-major float storage");
}

void sceneDataIsRendererIndependentAndValidatesMeshReferences()
{
    using namespace engine::core;

    SceneData scene;
    MeshData mesh;
    mesh.positions = {{-1.0f, -2.0f, 0.0f}, {3.0f, 2.0f, 0.0f}, {1.0f, 0.0f, 4.0f}};
    mesh.normals = {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};
    mesh.texCoords = {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f}};
    mesh.indices = {0, 1, 2};
    scene.meshes.push_back(mesh);

    MaterialData material;
    material.diffuseColor = {0.25f, 0.5f, 0.75f, 1.0f};
    material.opacity = 0.5f;
    material.smoothness = 0.75f;
    scene.materials.push_back(material);

    SceneNodeData root;
    scene.nodes.push_back(root);
    SceneNodeData model;
    model.parentIndex = 0;
    model.meshIndex = 0;
    model.materialIndex = 0;
    model.localTransform = Matrix4::translation({5.0f, 0.0f, -1.0f});
    scene.nodes.push_back(model);

    std::string error;
    check(scene.validate(error), "valid CPU scene data must be accepted");
    check(close(scene.materials[0].diffuseColor.y, 0.5f)
            && close(scene.materials[0].opacity, 0.5f)
            && close(scene.materials[0].smoothness, 0.75f),
        "material values must remain editable CPU-side values");

    const Bounds bounds = scene.meshes[0].bounds();
    check(close(bounds.minimum().x, -1.0f) && close(bounds.minimum().y, -2.0f)
            && close(bounds.minimum().z, 0.0f)
            && close(bounds.maximum().x, 3.0f) && close(bounds.maximum().y, 2.0f)
            && close(bounds.maximum().z, 4.0f),
        "mesh bounds must enclose all CPU positions");

    const Bounds worldBounds = bounds.transformed(
        Matrix4::scaling({-2.0f, 1.0f, 0.5f})
            * Matrix4::translation({5.0f, 0.0f, -1.0f}));
    check(close(worldBounds.minimum().x, -1.0f) && close(worldBounds.maximum().x, 7.0f)
            && close(worldBounds.minimum().z, -1.0f)
            && close(worldBounds.maximum().z, 1.0f),
        "transformed bounds must enclose mesh corners under negative scale");

    scene.meshes[0].indices[2] = 3;
    check(!scene.validate(error) && !error.empty(),
        "scene validation must reject indices outside the mesh vertex array");
}

} // namespace

int main()
{
    rowVectorTransformsAndProjectionUseDocumentedConventions();
    sceneDataIsRendererIndependentAndValidatesMeshReferences();
}
