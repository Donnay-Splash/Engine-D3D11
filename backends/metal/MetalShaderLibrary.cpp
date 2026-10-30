#include "MetalShaderLibrary.h"

#include "Core/Types.h"
#include "Graphics/Device.h"

#include <Foundation/Foundation.hpp>

#include <fstream>
#include <iterator>
#include <utility>

namespace Engine {
namespace Metal {

bool MetalShaderLibrary::Load(const std::string &entryPoint, std::vector<std::uint8_t> &bytecode,
                              Error &error)
{
    // File names follow the shader-compiler output convention: <Shader>_<entry>.metallib.
    const std::string fileName = "Scene_" + entryPoint;
    NS::SharedPtr<NS::String> name =
        NS::RetainPtr(NS::String::string(fileName.c_str(), NS::UTF8StringEncoding));
    NS::SharedPtr<NS::String> extension =
        NS::RetainPtr(NS::String::string("metallib", NS::UTF8StringEncoding));
    NS::SharedPtr<NS::String> subdirectory =
        NS::RetainPtr(NS::String::string("shaders", NS::UTF8StringEncoding));
    NS::SharedPtr<NS::Bundle> bundle = NS::RetainPtr(NS::Bundle::mainBundle());
    NS::SharedPtr<NS::URL> libraryURL = NS::RetainPtr(
        bundle->URLForResource(name.get(), extension.get(), subdirectory.get()));
    if (!libraryURL) {
        error.message = "A bundled Metal shader library is missing; run shader-compiler.";
        return false;
    }

    std::ifstream libraryFile(libraryURL->fileSystemRepresentation(), std::ios::binary);
    if (!libraryFile) {
        error.message = "Failed to open the bundled Metal shader library.";
        return false;
    }

    bytecode.assign(
        std::istreambuf_iterator<char>(libraryFile),
        std::istreambuf_iterator<char>());
    if (bytecode.empty()) {
        error.message = "The bundled Metal shader library is empty.";
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
    if (!Load("mesh_vertex", vertexBytecode, error) ||
        !Load("mesh_fragment", fragmentBytecode, error)) {
        return false;
    }

    Graphics::ShaderDesc vertexDescription;
    vertexDescription.stage = Graphics::ShaderStage::Vertex;
    vertexDescription.entryPoint = "mesh_vertex";
    vertexDescription.format = "metallib";
    vertexDescription.bytecode = std::move(vertexBytecode);
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
