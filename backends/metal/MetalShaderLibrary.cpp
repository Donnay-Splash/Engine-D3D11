#include "MetalShaderLibrary.h"

#include "Core/Types.h"
#include "Graphics/Device.h"

#include <Foundation/Foundation.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>

namespace Engine {
namespace Metal {

bool MetalShaderLibrary::Load(const std::string &entryPoint, const std::string &extension,
                              std::vector<std::uint8_t> &data, Error &error)
{
    // Files follow the shader-compiler output convention: <Shader>_<entry>.<extension>.
    const std::string fileName = "Scene_" + entryPoint;
    NS::SharedPtr<NS::Bundle> bundle = NS::RetainPtr(NS::Bundle::mainBundle());
    NS::String *resourcePath = bundle == nullptr ? nullptr : bundle->resourcePath();
    const char *resourceDirectory =
        resourcePath == nullptr ? nullptr : resourcePath->utf8String();
    if (resourceDirectory == nullptr) {
        error.message = "A bundled Metal shader artifact is missing; run shader-compiler.";
        return false;
    }

    const std::filesystem::path artifactPath =
        std::filesystem::path(resourceDirectory) / "shaders" / (fileName + "." + extension);
    std::ifstream libraryFile(artifactPath, std::ios::binary);
    if (!libraryFile) {
        error.message = "Failed to open the bundled Metal shader library.";
        return false;
    }

    data.assign(
        std::istreambuf_iterator<char>(libraryFile),
        std::istreambuf_iterator<char>());
    if (data.empty()) {
        error.message = "The bundled Metal shader artifact is empty.";
        return false;
    }
    return true;
}

bool MetalShaderLibrary::RequestGraphicsShaders(
    Graphics::Device &device,
    ShaderProgramType type,
    const std::string &name,
    GraphicsShaderPair &shaders,
    Error &error)
{
    if (type != ShaderProgramType::Mesh || name != "uv-triangle") {
        error.message = "The requested Metal graphics shader program is unsupported.";
        return false;
    }

    std::vector<std::uint8_t> vertexBytecode;
    std::vector<std::uint8_t> fragmentBytecode;
    std::vector<std::uint8_t> vertexReflection;
    std::vector<std::uint8_t> fragmentReflection;
    if (!Load("mesh_vertex", "metallib", vertexBytecode, error) ||
        !Load("mesh_vertex", "json", vertexReflection, error) ||
        !Load("mesh_fragment", "metallib", fragmentBytecode, error) ||
        !Load("mesh_fragment", "json", fragmentReflection, error)) {
        return false;
    }

    Graphics::ShaderDesc vertexDescription;
    vertexDescription.stage = Graphics::ShaderStage::Vertex;
    vertexDescription.entryPoint = "mesh_vertex";
    vertexDescription.format = "metallib";
    vertexDescription.bytecode = std::move(vertexBytecode);
    vertexDescription.reflection = std::move(vertexReflection);
    std::shared_ptr<Graphics::Shader> vertex =
        device.CreateShader(vertexDescription, error);
    if (vertex == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the Metal vertex shader.";
        }
        return false;
    }

    Graphics::ShaderDesc fragmentDescription;
    fragmentDescription.stage = Graphics::ShaderStage::Fragment;
    fragmentDescription.entryPoint = "mesh_fragment";
    fragmentDescription.format = "metallib";
    fragmentDescription.bytecode = std::move(fragmentBytecode);
    fragmentDescription.reflection = std::move(fragmentReflection);
    std::shared_ptr<Graphics::Shader> fragment =
        device.CreateShader(fragmentDescription, error);
    if (fragment == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the Metal fragment shader.";
        }
        return false;
    }

    shaders.vertex = std::move(vertex);
    shaders.fragment = std::move(fragment);
    return true;
}

} // namespace Metal
} // namespace Engine
