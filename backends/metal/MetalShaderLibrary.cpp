#include "MetalShaderLibrary.h"

#include "Core/Types.h"
#include "Graphics/Device.h"

#include <Foundation/Foundation.hpp>

#include <fstream>
#include <iterator>
#include <utility>

namespace Engine {
namespace Metal {

bool MetalShaderLibrary::Load(Error &error)
{
    NS::SharedPtr<NS::String> name =
        NS::RetainPtr(NS::String::string("default", NS::UTF8StringEncoding));
    NS::SharedPtr<NS::String> extension =
        NS::RetainPtr(NS::String::string("metallib", NS::UTF8StringEncoding));
    NS::SharedPtr<NS::Bundle> bundle = NS::RetainPtr(NS::Bundle::mainBundle());
    NS::SharedPtr<NS::URL> libraryURL =
        NS::RetainPtr(bundle->URLForResource(name.get(), extension.get()));
    if (!libraryURL) {
        error.message = "The bundled Metal shader library is missing.";
        return false;
    }

    std::ifstream libraryFile(libraryURL->fileSystemRepresentation(), std::ios::binary);
    if (!libraryFile) {
        error.message = "Failed to open the bundled Metal shader library.";
        return false;
    }

    m_ShaderBytecode.assign(
        std::istreambuf_iterator<char>(libraryFile),
        std::istreambuf_iterator<char>());
    if (m_ShaderBytecode.empty()) {
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

    if (m_ShaderBytecode.empty() && !Load(error)) {
        return false;
    }

    Graphics::ShaderDesc vertexDescription;
    vertexDescription.stage = Graphics::ShaderStage::Vertex;
    vertexDescription.entryPoint = "mesh_vertex";
    vertexDescription.format = "metallib";
    vertexDescription.bytecode = m_ShaderBytecode;
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
    fragmentDescription.bytecode = m_ShaderBytecode;
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
