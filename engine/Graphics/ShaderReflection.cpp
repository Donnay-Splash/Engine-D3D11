#include "Graphics/ShaderReflection.h"

#include "Core/Types.h"

#include <nlohmann/json.hpp>

#include <utility>

namespace Engine {
namespace Graphics {
namespace {

bool parseStage(const std::string &text, ShaderStage &stage)
{
    if (text == "vertex") {
        stage = ShaderStage::Vertex;
    } else if (text == "fragment") {
        stage = ShaderStage::Fragment;
    } else if (text == "compute") {
        stage = ShaderStage::Compute;
    } else {
        return false;
    }
    return true;
}

bool parseFormat(const std::string &text, VertexFormat &format)
{
    if (text == "float2") {
        format = VertexFormat::Float2;
    } else if (text == "float3") {
        format = VertexFormat::Float3;
    } else if (text == "float4") {
        format = VertexFormat::Float4;
    } else if (text == "uint") {
        format = VertexFormat::UInt32;
    } else {
        return false;
    }
    return true;
}

bool parseBindingType(const std::string &text, BindingType &type)
{
    if (text == "uniformBuffer") {
        type = BindingType::UniformBuffer;
    } else if (text == "storageBuffer") {
        type = BindingType::StorageBuffer;
    } else if (text == "sampledTexture") {
        type = BindingType::SampledTexture;
    } else if (text == "storageTexture") {
        type = BindingType::StorageTexture;
    } else if (text == "sampler") {
        type = BindingType::Sampler;
    } else {
        return false;
    }
    return true;
}

} // namespace

bool ParseShaderReflection(std::string_view json, ShaderReflection &reflection, Error &error)
{
    nlohmann::json root = nlohmann::json::parse(json.begin(), json.end(), nullptr, false);
    if (!root.is_object()) {
        error.message = "The shader reflection is not a valid JSON object.";
        return false;
    }

    ShaderReflection parsed;
    try {
        if (!parseStage(root.at("stage").get<std::string>(), parsed.stage)) {
            error.message = "The shader reflection has an unknown stage.";
            return false;
        }
        for (const nlohmann::json &input : root.at("vertexInputs")) {
            ShaderVertexInput vertexInput;
            vertexInput.semantic = input.at("semantic").get<std::string>();
            vertexInput.location = input.at("location").get<std::uint32_t>();
            if (!parseFormat(input.at("format").get<std::string>(), vertexInput.format)) {
                error.message = "The shader reflection has an unknown vertex format.";
                return false;
            }
            parsed.vertexInputs.push_back(std::move(vertexInput));
        }
        for (const nlohmann::json &entry : root.at("bindings")) {
            BindingDesc binding;
            binding.binding = entry.at("binding").get<std::uint32_t>();
            binding.arrayCount = entry.at("arrayCount").get<std::uint32_t>();
            binding.visibility = parsed.stage;
            if (!parseBindingType(entry.at("type").get<std::string>(), binding.type)) {
                error.message = "The shader reflection has an unknown binding type.";
                return false;
            }
            parsed.bindings.push_back(binding);
        }
    } catch (const nlohmann::json::exception &exception) {
        error.message = std::string("The shader reflection is malformed: ") + exception.what();
        return false;
    }
    reflection = std::move(parsed);
    return true;
}

} // namespace Graphics
} // namespace Engine
