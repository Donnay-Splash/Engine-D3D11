#import <Foundation/Foundation.h>

#include "MetalShaderLibrary.h"

#include "Core/Types.h"
#include "Graphics/Device.h"

#include <utility>

namespace Engine {
namespace Metal {

bool MetalShaderLibrary::Load(Error &error)
{
    NSURL *libraryURL = [NSBundle.mainBundle URLForResource:@"default" withExtension:@"metallib"];
    if (libraryURL == nil) {
        error.message = "The bundled Metal shader library is missing.";
        return false;
    }

    NSError *libraryError = nil;
    NSData *libraryData = [NSData dataWithContentsOfURL:libraryURL options:0 error:&libraryError];
    if (libraryData == nil || libraryData.length == 0) {
        error.message = libraryError.localizedDescription.UTF8String
            ?: "Failed to read the bundled Metal shader library.";
        return false;
    }

    const std::uint8_t *bytes = static_cast<const std::uint8_t *>(libraryData.bytes);
    m_ShaderBytecode.assign(bytes, bytes + libraryData.length);
    return true;
}

bool MetalShaderLibrary::RequestGraphicsShaders(
    Graphics::Device &device,
    ShaderProgramType type,
    const std::string &name,
    GraphicsShaderPair &shaders,
    Error &error)
{
    (void)type;
    (void)name;

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
