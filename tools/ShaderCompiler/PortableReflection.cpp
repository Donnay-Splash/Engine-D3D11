#include "PortableReflection.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>

namespace ShaderCompiler {
namespace {

// Size of one resource descriptor in the Metal top-level argument buffer.
constexpr unsigned kDescriptorSize = 24;

bool VertexFormat(const std::string &elementType, unsigned columns, std::string &format)
{
    if (elementType == "Float" && columns >= 2 && columns <= 4) {
        format = "float" + std::to_string(columns);
        return true;
    }
    if (elementType == "UInt" && columns == 1) {
        format = "uint";
        return true;
    }
    return false;
}

// Metal reports CBV, SRV, UAV and SMP. SRV does not say texture or buffer, so it maps to a
// sampled texture until DXC reflection is used instead.
bool BindingType(const std::string &type, std::string &portable)
{
    if (type == "CBV") {
        portable = "uniformBuffer";
    } else if (type == "SRV") {
        portable = "sampledTexture";
    } else if (type == "UAV") {
        portable = "storageBuffer";
    } else if (type == "SMP") {
        portable = "sampler";
    } else {
        return false;
    }
    return true;
}

} // namespace

bool BuildPortableReflection(const std::string &metalReflectionJson, const std::string &stage,
                             std::string &portableJson, std::string &error)
{
    const nlohmann::json metal = nlohmann::json::parse(metalReflectionJson, nullptr, false);
    if (!metal.is_object()) {
        error = "The Metal reflection is not valid JSON.";
        return false;
    }

    nlohmann::json portable;
    portable["stage"] = stage == "vs" ? "vertex" : stage == "ps" ? "fragment" : "compute";
    portable["vertexInputs"] = nlohmann::json::array();
    portable["bindings"] = nlohmann::json::array();

    try {
        if (metal.contains("state") && metal["state"].contains("vertex_inputs")) {
            for (const nlohmann::json &input : metal["state"]["vertex_inputs"]) {
                std::string format;
                if (!VertexFormat(input.at("elementType").get<std::string>(),
                                  input.at("columnCount").get<unsigned>(), format)) {
                    error = "Unsupported vertex input type for " +
                            input.at("name").get<std::string>() + ".";
                    return false;
                }
                std::string semantic = input.at("name").get<std::string>();
                std::transform(semantic.begin(), semantic.end(), semantic.begin(),
                               [](unsigned char c) { return std::toupper(c); });
                portable["vertexInputs"].push_back({{"semantic", semantic},
                                                    {"format", format},
                                                    {"location", input.at("index")}});
            }
        }
        if (metal.contains("TopLevelArgumentBuffer")) {
            for (const nlohmann::json &entry : metal["TopLevelArgumentBuffer"]) {
                std::string type;
                const unsigned size = entry.at("Size").get<unsigned>();
                if (entry.at("Space").get<unsigned>() != 0 ||
                    !BindingType(entry.at("Type").get<std::string>(), type) ||
                    size == 0 || size % kDescriptorSize != 0) {
                    error = "Unsupported resource binding in the Metal reflection.";
                    return false;
                }
                portable["bindings"].push_back({{"binding", entry.at("Slot")},
                                                {"type", type},
                                                {"arrayCount", size / kDescriptorSize},
                                                {"name", entry.at("Name")}});
            }
        }
    } catch (const nlohmann::json::exception &exception) {
        error = std::string("The Metal reflection is malformed: ") + exception.what();
        return false;
    }

    portableJson = portable.dump(2) + "\n";
    return true;
}

} // namespace ShaderCompiler
