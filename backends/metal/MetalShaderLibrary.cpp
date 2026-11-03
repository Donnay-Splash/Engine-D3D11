#include "MetalShaderLibrary.h"

#include "Core/Types.h"
#include "Graphics/Device.h"

#include <Foundation/Foundation.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string_view>
#include <utility>

namespace Engine {
namespace Metal {

bool MetalShaderLibrary::Load(const std::string &fileName, std::vector<std::uint8_t> &data,
                              Error &error)
{
    NS::SharedPtr<NS::Bundle> bundle = NS::RetainPtr(NS::Bundle::mainBundle());
    NS::String *resourcePath = bundle.get() == nullptr ? nullptr : bundle->resourcePath();
    const char *resourceDirectory =
        resourcePath == nullptr ? nullptr : resourcePath->utf8String();
    if (resourceDirectory == nullptr) {
        error.message = "A bundled Metal shader artifact is missing; run shader-compiler.";
        return false;
    }

    const std::filesystem::path artifactPath =
        std::filesystem::path(resourceDirectory) / "shaders" / fileName;
    std::ifstream file(artifactPath, std::ios::binary);
    if (!file) {
        error.message = "Failed to open the bundled shader artifact " + fileName
            + "; run shader-compiler.";
        return false;
    }

    data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    if (data.empty()) {
        error.message = "The bundled shader artifact " + fileName + " is empty.";
        return false;
    }
    return true;
}

bool MetalShaderLibrary::LoadProgramConfig(std::string &text, Error &error)
{
    std::vector<std::uint8_t> data;
    if (!Load("ShaderPrograms.cfg", data, error)) {
        return false;
    }
    text.assign(data.begin(), data.end());
    return true;
}

bool MetalShaderLibrary::RequestShader(
    Graphics::Device &device,
    const ShaderRef &ref,
    Graphics::ShaderStage stage,
    LoadedShader &loaded,
    Error &error)
{
    // Files follow the shader-compiler output convention: <Source>_<entry>.<extension>.
    const std::string baseName = ref.source + "_" + ref.entry;
    Graphics::ShaderDesc description;
    description.stage = stage;
    description.entryPoint = ref.entry;
    description.format = "metallib";
    std::vector<std::uint8_t> portableReflection;
    if (!Load(baseName + ".metallib", description.bytecode, error)
        || !Load(baseName + ".json", description.reflection, error)
        || !Load(baseName + ".reflection.json", portableReflection, error)) {
        return false;
    }

    Graphics::ShaderReflection reflection;
    if (!Graphics::ParseShaderReflection(
            std::string_view(
                reinterpret_cast<const char *>(portableReflection.data()),
                portableReflection.size()),
            reflection, error)) {
        return false;
    }

    std::shared_ptr<Graphics::Shader> shader = device.CreateShader(description, error);
    if (shader == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the Metal shader " + baseName + ".";
        }
        return false;
    }
    loaded.shader = std::move(shader);
    loaded.reflection = std::move(reflection);
    return true;
}

} // namespace Metal
} // namespace Engine
