#include "Core/Math.h"
#include "Core/Mesh.h"
#include "Core/Model.h"
#include "Core/ModelInstance.h"
#include "Core/Scene.h"
#include "Core/Types.h"
#include "Graphics/Buffer.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>
#include <cstddef>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace {

class TestBuffer final : public Engine::Graphics::Buffer
{
public:
    explicit TestBuffer(const Engine::Graphics::BufferDesc &description)
        : description_(description)
    {
    }

    const Engine::Graphics::BufferDesc &Description() const noexcept override
    {
        return description_;
    }

    bool Map(void *&data, Engine::Error &) override
    {
        data = nullptr;
        return false;
    }

    void Unmap() noexcept override {}

private:
    Engine::Graphics::BufferDesc description_;
};

void check(bool condition, const char *message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void meshRequiresValidMatchingVertexStreams()
{
    using namespace Engine::Graphics;
    Engine::Error error;
    Engine::Mesh mesh;

    BufferDesc positionDescription;
    positionDescription.size = sizeof(float) * 9;
    positionDescription.usage = BufferUsage::Vertex;
    Engine::VertexStream positions;
    positions.slot = 0;
    positions.format = VertexFormat::Float3;
    positions.stride = sizeof(float) * 3;
    positions.vertexCount = 3;
    positions.buffer = std::make_shared<TestBuffer>(positionDescription);
    check(mesh.AddVertexStream(positions, error), "valid position stream should be accepted");

    BufferDesc uvDescription;
    uvDescription.size = sizeof(float) * 4;
    uvDescription.usage = BufferUsage::Vertex;
    Engine::VertexStream uvs;
    uvs.slot = 1;
    uvs.format = VertexFormat::Float2;
    uvs.stride = sizeof(float) * 2;
    uvs.vertexCount = 2;
    uvs.buffer = std::make_shared<TestBuffer>(uvDescription);
    check(!mesh.AddVertexStream(uvs, error), "mismatched stream count should be rejected");

    error.message.clear();
    uvs.vertexCount = 3;
    uvDescription.size = sizeof(float) * 6;
    uvs.buffer = std::make_shared<TestBuffer>(uvDescription);
    check(mesh.AddVertexStream(uvs, error), "matching UV stream should be accepted");
    check(mesh.VertexCount() == 3, "mesh count should follow its synchronized streams");
}

bool close(float actual, float expected)
{
    return std::fabs(actual - expected) < 0.0001f;
}

void rowVectorTransformsAndProjectionUseDocumentedConventions()
{
    using Engine::Maths::Matrix4;
    using Engine::Maths::Quaternion;
    using Engine::Maths::Vector4;

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

void materialInstancesOverrideOnlyTheirOwnParameters()
{
    Engine::Error error;
    auto material = Engine::Material::CreateFlatColor("flat", {0.1f, 0.2f, 0.3f, 1.0f});
    Engine::MaterialInstance first(material);
    Engine::MaterialInstance second(material);

    check(!first.SetVector4("missing", {}, error) && !error.message.empty(),
        "unknown parameters should be rejected");
    check(first.SetVector4(Engine::Material::kBaseColor, {1.0f, 0.0f, 0.0f, 1.0f}, error),
        "known parameters should be overridable");
    check(first.IsOverridden(Engine::Material::kBaseColor)
            && !second.IsOverridden(Engine::Material::kBaseColor),
        "overrides should be per instance");
    check(first.GetVector4(Engine::Material::kBaseColor)->x == 1.0f
            && second.GetVector4(Engine::Material::kBaseColor)->x == 0.1f,
        "instances should fall back to material defaults");
    first.ClearOverride(Engine::Material::kBaseColor);
    check(first.ResolveParameters().front().x == 0.1f, "clearing should restore the default");
}

void modelInstancesComposeModelsMaterialsAndPlacement()
{
    Engine::Error error;
    Engine::Model model;
    auto material = Engine::Material::CreateFlatColor("flat", {1.0f, 1.0f, 1.0f, 1.0f});
    check(!model.AddPart({nullptr, material}, error), "parts require a mesh");
    auto mesh = std::make_shared<Engine::Mesh>();
    Engine::Maths::Bounds bounds;
    bounds.include({-1.0f, 0.0f, 0.0f});
    bounds.include({1.0f, 2.0f, 0.0f});
    mesh->SetLocalBounds(bounds);
    check(model.AddPart({mesh, material}, error) && model.AddPart({mesh, material}, error),
        "valid parts should be accepted");
    check(model.Parts().size() == 2 && model.LocalBounds().maximum().y == 2.0f,
        "model should aggregate parts and bounds");

    Engine::ModelInstance instance(std::make_shared<Engine::Model>(model));
    instance.SetPosition({3.0f, 4.0f, 5.0f});
    const Engine::Maths::Matrix4 world = instance.WorldMatrix();
    check(close(world.transform({0.0f, 0.0f, 0.0f, 1.0f}).x, 3.0f)
            && close(world.transform({0.0f, 0.0f, 0.0f, 1.0f}).z, 5.0f),
        "world matrix should translate by the instance position");
    check(instance.GetMaterialInstance(0).GetMaterial() == material,
        "each part should get a material instance");

    Engine::Scene scene;
    auto shared = std::make_shared<Engine::ModelInstance>(std::make_shared<Engine::Model>(model));
    scene.Add(shared);
    check(scene.Instances().size() == 1 && scene.Remove(shared) && !scene.Remove(shared),
        "scene should add and remove instances");
}

} // namespace

int main()
{
    materialInstancesOverrideOnlyTheirOwnParameters();
    modelInstancesComposeModelsMaterialsAndPlacement();
    meshRequiresValidMatchingVertexStreams();
    rowVectorTransformsAndProjectionUseDocumentedConventions();
}
