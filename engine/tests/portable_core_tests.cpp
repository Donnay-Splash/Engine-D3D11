#include "Core/Material.h"
#include "Core/MaterialInstance.h"
#include "Core/MaterialLayout.h"
#include "Core/Math.h"
#include "Core/Mesh.h"
#include "Core/Model.h"
#include "Core/ModelInstance.h"
#include "Core/Scene.h"
#include "Core/Types.h"
#include "Graphics/Buffer.h"
#include "Graphics/ShaderReflection.h"
#include "Rendering/ImageLoader.h"
#include "ShaderProgramCatalog.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>
#include <cstddef>
#include <cstring>
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
    positions.semantic = "POSITION0";
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
    uvs.semantic = "TEXCOORD0";
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

constexpr const char *kMaterialReflectionJson = R"({
  "stage": "fragment", "vertexInputs": [], "bindings": [
    {"binding": 0, "set": 0, "type": "uniformBuffer", "arrayCount": 1, "name": "Frame"},
    {"binding": 0, "set": 1, "type": "sampledTexture", "arrayCount": 1, "name": "baseTexture"},
    {"binding": 0, "set": 1, "type": "uniformBuffer", "arrayCount": 1,
     "name": "MaterialParams", "size": 40,
     "members": [
       {"name": "useTexture", "type": "bool", "offset": 0, "size": 4},
       {"name": "mode", "type": "int", "offset": 4, "size": 4},
       {"name": "intensity", "type": "float", "offset": 8, "size": 4},
       {"name": "tint", "type": "float4", "offset": 16, "size": 16},
       {"name": "uvScale", "type": "float2", "offset": 32, "size": 8}]}]})";

std::shared_ptr<const Engine::MaterialLayout> buildTestLayout()
{
    Engine::Graphics::ShaderReflection reflection;
    Engine::Error error;
    check(Engine::Graphics::ParseShaderReflection(kMaterialReflectionJson, reflection, error),
        "material reflection should parse");
    std::shared_ptr<const Engine::MaterialLayout> layout;
    check(Engine::MaterialLayout::Build({&reflection}, layout, error) && layout != nullptr,
        "a material layout should build from reflection");
    return layout;
}

void materialLayoutIsDerivedFromReflectionAndPacksValues()
{
    const auto layout = buildTestLayout();
    check(layout->Parameters().size() == 6 && layout->BufferSize() == 40,
        "every material-set member and texture should become a parameter");
    check(layout->Parameters()[layout->Find("tint").value()].type
            == Engine::MaterialValueType::Float4
            && layout->Parameters()[layout->Find("baseTexture").value()].type
                == Engine::MaterialValueType::Texture
            && !layout->Find("Frame"),
        "parameter types come from reflection and engine-set resources are excluded");

    Engine::Error error;
    Engine::MaterialInstance instance(std::make_shared<Engine::Material>("test", layout));
    check(instance.SetBool("useTexture", true, error)
            && instance.SetInt("mode", -2, error)
            && instance.SetFloat("intensity", 0.5f, error)
            && instance.SetVector4("tint", {1.0f, 2.0f, 3.0f, 4.0f}, error)
            && instance.SetVector2("uvScale", {5.0f, 6.0f}, error)
            && instance.SetTexture("baseTexture", nullptr, error),
        "each value type should be settable");

    const std::vector<std::uint8_t> bytes = layout->Pack(instance.ResolveValues());
    float floats[10] = {};
    std::int32_t ints[10] = {};
    std::memcpy(floats, bytes.data(), sizeof(floats));
    std::memcpy(ints, bytes.data(), sizeof(ints));
    check(bytes.size() == 40 && ints[0] == 1 && ints[1] == -2 && floats[2] == 0.5f
            && floats[4] == 1.0f && floats[7] == 4.0f && floats[8] == 5.0f && floats[9] == 6.0f,
        "values should be packed at their reflected offsets");
}

void materialInstancesOverrideOnlyTheirOwnParameters()
{
    Engine::Error error;
    auto material = std::make_shared<Engine::Material>("test", buildTestLayout());
    Engine::MaterialInstance first(material);
    Engine::MaterialInstance second(material);

    check(!first.SetFloat("missing", 1.0f, error) && !error.message.empty(),
        "unknown parameters should be rejected");
    check(!first.SetInt("intensity", 1, error), "values of the wrong type should be rejected");
    const std::uint64_t before = first.Version();
    check(first.SetVector4("tint", {1.0f, 0.0f, 0.0f, 1.0f}, error),
        "known parameters should be overridable");
    check(first.Version() > before, "changes should bump the version");
    check(first.IsOverridden("tint") && !second.IsOverridden("tint"),
        "overrides should be per instance");
    check(std::get<Engine::Maths::Vector4>(*first.Get("tint")).x == 1.0f
            && std::get<Engine::Maths::Vector4>(*second.Get("tint")).x == 0.0f,
        "instances should fall back to layout defaults");
    first.ClearOverride("tint");
    check(!first.IsOverridden("tint"), "clearing should restore the default");
}

constexpr const char *kDefaultsReflectionJson = R"({
  "stage": "fragment", "vertexInputs": [], "bindings": [
    {"binding": 1, "set": 1, "type": "sampledTexture", "arrayCount": 1, "name": "normalMap",
     "default": "normal"},
    {"binding": 0, "set": 1, "type": "uniformBuffer", "arrayCount": 1,
     "name": "MaterialParams", "size": 32,
     "members": [
       {"name": "enabled", "type": "bool", "offset": 0, "size": 4, "default": [1]},
       {"name": "count", "type": "int", "offset": 4, "size": 4, "default": [3]},
       {"name": "tint", "type": "float4", "offset": 16, "size": 16, "default": [1, 0.5, 0.25, 1]}]}]})";

void materialDefaultsComeFromReflection()
{
    Engine::Graphics::ShaderReflection reflection;
    Engine::Error error;
    check(Engine::Graphics::ParseShaderReflection(kDefaultsReflectionJson, reflection, error),
        "reflection with defaults should parse");
    std::shared_ptr<const Engine::MaterialLayout> layout;
    check(Engine::MaterialLayout::Build({&reflection}, layout, error), "layout should build");
    Engine::MaterialInstance instance(std::make_shared<Engine::Material>("test", layout));
    check(std::get<bool>(*instance.Get("enabled")) && std::get<std::int32_t>(*instance.Get("count")) == 3
            && std::get<Engine::Maths::Vector4>(*instance.Get("tint")).y == 0.5f,
        "unset parameters should use the shader's defaults");
    check(layout->Parameters()[layout->Find("normalMap").value()].defaultTexture
            == Engine::DefaultTexture::FlatNormal,
        "texture defaults should come from reflection");

    std::string wrongSize = kDefaultsReflectionJson;
    wrongSize.replace(wrongSize.find("[1, 0.5, 0.25, 1]"), 17, "[1, 0.5]");
    check(!Engine::Graphics::ParseShaderReflection(wrongSize, reflection, error),
        "a default with the wrong component count should be rejected");
}

void materialLayoutRejectsInconsistentStages()
{
    Engine::Graphics::ShaderReflection vertex;
    Engine::Graphics::ShaderReflection fragment;
    Engine::Error error;
    check(Engine::Graphics::ParseShaderReflection(kMaterialReflectionJson, fragment, error),
        "fragment reflection should parse");
    std::string mismatched = kMaterialReflectionJson;
    mismatched.replace(mismatched.find("\"stage\": \"fragment\""), 19, "\"stage\": \"vertex\"");
    mismatched.replace(mismatched.find("\"offset\": 4"), 11, "\"offset\": 8");
    check(Engine::Graphics::ParseShaderReflection(mismatched, vertex, error),
        "vertex reflection should parse");
    std::shared_ptr<const Engine::MaterialLayout> layout;
    check(!Engine::MaterialLayout::Build({&vertex, &fragment}, layout, error),
        "stages that disagree on a parameter should be rejected");
    check(Engine::MaterialLayout::Build({&fragment, &fragment}, layout, error)
            && layout->Parameters().size() == 6,
        "stages that agree should merge");

    Engine::Graphics::ShaderReflection plain;
    check(Engine::Graphics::ParseShaderReflection(
              R"({"stage": "vertex", "vertexInputs": [], "bindings": [
                  {"binding": 0, "type": "uniformBuffer", "arrayCount": 1, "name": "F"}]})",
              plain, error)
            && Engine::MaterialLayout::Build({&plain}, layout, error) && layout == nullptr,
        "programs without material resources have no layout");
}

void modelInstancesComposeModelsMaterialsAndPlacement()
{
    Engine::Error error;
    Engine::Model model;
    auto material = std::make_shared<Engine::Material>("flat");
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

void shaderProgramCatalogParsesSourceAndEntryPerStage()
{
    Engine::ShaderProgramCatalog catalog;
    Engine::Error error;
    check(Engine::ShaderProgramCatalog::Parse(
              "# comment\n[program]\nname = Mesh\nvertex = A:vs_main  # trailing\n"
              "fragment = B:ps_main\n[program]\nname = Depth\nvertex = A:vs_depth\n",
              catalog, error),
        "a valid program config should parse");
    const Engine::ShaderProgramDesc *mesh = catalog.Find("Mesh");
    check(mesh != nullptr && mesh->vertex && mesh->vertex->source == "A" && mesh->vertex->entry == "vs_main"
            && mesh->fragment && mesh->fragment->source == "B"
            && mesh->fragment->entry == "ps_main",
        "stages may come from different sources");
    const Engine::ShaderProgramDesc *depth = catalog.Find("Depth");
    check(depth != nullptr && !depth->fragment, "the fragment stage is optional");
    check(catalog.Find("Missing") == nullptr, "unknown programs are not found");

    Engine::ShaderProgramCatalog rejected;
    check(!Engine::ShaderProgramCatalog::Parse("[program]\nname = X\nvertex = nocolon\n",
              rejected, error),
        "a stage without source:entry should be rejected");
    check(!Engine::ShaderProgramCatalog::Parse("[program]\nname = X\n", rejected, error),
        "a program without a vertex stage should be rejected");
    check(!Engine::ShaderProgramCatalog::Parse(
              "[program]\nname = X\nvertex = A:b\n[program]\nname = X\nvertex = A:c\n",
              rejected, error),
        "duplicate program names should be rejected");
}

void shaderReflectionParsesInputsAndBindings()
{
    using namespace Engine::Graphics;
    ShaderReflection reflection;
    Engine::Error error;
    check(ParseShaderReflection(
              R"({"stage": "vertex",
                  "vertexInputs": [{"semantic": "POSITION0", "format": "float3", "location": 0}],
                  "bindings": [{"binding": 2, "type": "storageBuffer", "arrayCount": 4,
                                "name": ""}]})",
              reflection, error),
        "valid reflection should parse");
    check(reflection.stage == ShaderStage::Vertex && reflection.vertexInputs.size() == 1
            && reflection.vertexInputs[0].semantic == "POSITION0"
            && reflection.vertexInputs[0].format == VertexFormat::Float3,
        "vertex inputs should be parsed");
    check(reflection.bindings.size() == 1 && reflection.bindings[0].binding == 2
            && reflection.bindings[0].type == BindingType::StorageBuffer
            && reflection.bindings[0].arrayCount == 4
            && reflection.bindings[0].visibility == ShaderStage::Vertex,
        "bindings should take the shader's stage as visibility");
    check(ParseShaderReflection(
              R"({"stage": "fragment", "vertexInputs": [], "bindings": [
                  {"binding": 0, "set": 1, "type": "uniformBuffer", "arrayCount": 1,
                   "name": "MaterialParams", "size": 24,
                   "members": [{"name": "tint", "type": "float4", "offset": 0, "size": 16},
                               {"name": "mode", "type": "int", "offset": 16, "size": 4}]}]})",
              reflection, error),
        "reflection with a uniform block should parse");
    check(reflection.bindings.size() == 1 && reflection.bindings[0].set == 1
            && reflection.bindings[0].name == "MaterialParams"
            && reflection.uniformBlocks.size() == 1
            && reflection.uniformBlocks[0].size == 24
            && reflection.uniformBlocks[0].members.size() == 2
            && reflection.uniformBlocks[0].members[1].type == ShaderValueType::Int
            && reflection.uniformBlocks[0].members[1].offset == 16,
        "sets, names and buffer members should be parsed");
    check(!ParseShaderReflection(
              R"({"stage": "fragment", "vertexInputs": [], "bindings": [
                  {"binding": 0, "type": "uniformBuffer", "arrayCount": 1, "name": "M",
                   "size": 4, "members": [{"name": "x", "type": "float4", "offset": 0,
                                           "size": 16}]}]})",
              reflection, error),
        "members outside the buffer are rejected");
    check(!ParseShaderReflection("not json", reflection, error), "invalid JSON is rejected");
    check(!ParseShaderReflection(
              R"({"stage": "vertex", "vertexInputs": [], "bindings": [
                  {"binding": 0, "type": "bogus", "arrayCount": 1, "name": ""}]})",
              reflection, error),
        "unknown binding types are rejected");
}

} // namespace

void imageLoaderDecodesToRgba8()
{
    // 2x2 RGBA PNG: red, green / blue, half-transparent white.
    const std::uint8_t png[] = {
        0x89, 0x50, 0x4e, 0x47, 0xd, 0xa, 0x1a, 0xa, 0x0, 0x0, 0x0, 0xd, 0x49, 0x48, 0x44, 0x52,
        0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x2, 0x8, 0x6, 0x0, 0x0, 0x0, 0x72, 0xb6, 0xd, 0x24,
        0x0, 0x0, 0x0, 0x13, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
        0x1f, 0xc, 0x81, 0x34, 0x8, 0x34, 0x0, 0x0, 0x49, 0x49, 0x9, 0x78, 0x28, 0xa0, 0xdb,
        0x77, 0x0, 0x0, 0x0, 0x0, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
    Engine::Image image;
    Engine::Error error;
    check(Engine::ImageLoader::Decode(png, sizeof(png), image, error), "valid PNG should decode");
    check(image.width == 2 && image.height == 2 && image.pixels.size() == 16,
            "decoded image should be 2x2 RGBA8");
    check(image.pixels[0] == 255 && image.pixels[1] == 0 && image.pixels[4] == 0
                && image.pixels[5] == 255 && image.pixels[8] == 0 && image.pixels[10] == 255
                && image.pixels[15] == 128,
            "decoded pixels should keep their order and alpha");
    Engine::Error badError;
    const std::uint8_t garbage[] = {1, 2, 3, 4};
    check(!Engine::ImageLoader::Decode(garbage, sizeof(garbage), image, badError)
                && !badError.message.empty(),
            "invalid data should fail with a message");
}

int main()
{
    shaderProgramCatalogParsesSourceAndEntryPerStage();
    imageLoaderDecodesToRgba8();
    shaderReflectionParsesInputsAndBindings();
    materialLayoutIsDerivedFromReflectionAndPacksValues();
    materialInstancesOverrideOnlyTheirOwnParameters();
    materialDefaultsComeFromReflection();
    materialLayoutRejectsInconsistentStages();
    modelInstancesComposeModelsMaterialsAndPlacement();
    meshRequiresValidMatchingVertexStreams();
    rowVectorTransformsAndProjectionUseDocumentedConventions();
}
