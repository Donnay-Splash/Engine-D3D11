#include "PortableReflection.h"

#include "DefaultAnnotations.h"
#include "DxcBufferLayout.h"

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
    } else if (type == "SMP" || type == "Sampler") {
        portable = "sampler";
    } else {
        return false;
    }
    return true;
}

// Scalar and vector types a material parameter can have. Returns the byte size, or 0.
unsigned MemberType(const std::string &hlslType, std::string &portable)
{
    if (hlslType == "bool" || hlslType == "int" || hlslType == "float") {
        portable = hlslType;
        return 4;
    }
    if (hlslType == "float2" || hlslType == "float3" || hlslType == "float4") {
        portable = hlslType;
        return 4 * static_cast<unsigned>(hlslType.back() - '0');
    }
    return 0;
}

std::vector<DefaultAnnotation>::const_iterator FindAnnotation(
    const std::vector<DefaultAnnotation> &defaults, const std::string &name)
{
    return std::find_if(defaults.begin(), defaults.end(),
                        [&name](const DefaultAnnotation &item) { return item.name == name; });
}

// Members of an engine-owned buffer (set 0) may be anything; material buffers (set 1 and up)
// must be scalars and vectors so the engine can expose them as parameters.
bool BuildMembers(const DxcBufferBlock &block, unsigned set,
                  const std::vector<DefaultAnnotation> &defaults, nlohmann::json &members,
                  std::string &error)
{
    members = nlohmann::json::array();
    for (const DxcBufferMember &member : block.members) {
        std::string type;
        const unsigned size = MemberType(member.type, type);
        if (size == 0) {
            if (set == 0) {
                members = nlohmann::json::array();
                return true;
            }
            error = "Unsupported material parameter type " + member.type + " for " + block.name +
                    "." + member.name + ".";
            return false;
        }
        nlohmann::json entry = {
            {"name", member.name}, {"type", type}, {"offset", member.offset}, {"size", size}};
        const auto annotation = FindAnnotation(defaults, member.name);
        if (annotation != defaults.end() && !BuildValueDefault(*annotation, type, entry["default"], error)) {
            return false;
        }
        members.push_back(std::move(entry));
    }
    return true;
}

// The dxc "Type" column for each Metal resource class.
bool KindMatches(const std::string &metalType, const std::string &dxcKind)
{
    return (metalType == "CBV" && dxcKind == "cbuffer") ||
           (metalType == "SRV" && dxcKind == "texture") ||
           (metalType == "UAV" && dxcKind == "UAV") ||
           ((metalType == "SMP" || metalType == "Sampler") && dxcKind == "sampler");
}

} // namespace

bool BuildPortableReflection(const std::string &metalReflectionJson, const std::string &stage,
                             const std::string &dxcDisassembly,
                             const std::vector<DefaultAnnotation> &defaults,
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

    const std::vector<DxcBufferBlock> blocks = ParseDxcBufferBlocks(dxcDisassembly);
    const std::vector<DxcResourceBinding> resources = ParseDxcResourceBindings(dxcDisassembly);
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
                const unsigned space = entry.at("Space").get<unsigned>();
                if (!BindingType(entry.at("Type").get<std::string>(), type) ||
                    size == 0 || size % kDescriptorSize != 0) {
                    error = "Unsupported resource binding in the Metal reflection.";
                    return false;
                }
                const unsigned slot = entry.at("Slot").get<unsigned>();
                const std::string metalType = entry.at("Type").get<std::string>();
                const auto resource = std::find_if(
                    resources.begin(), resources.end(), [&](const DxcResourceBinding &item) {
                        return item.slot == slot && item.space == space &&
                               KindMatches(metalType, item.kind);
                    });
                if (resource == resources.end()) {
                    error = "No declared name found for a " + metalType + " at register " +
                            std::to_string(slot) + ", space " + std::to_string(space) + ".";
                    return false;
                }
                const std::string name = resource->name;
                nlohmann::json binding = {{"binding", slot},
                                          {"set", space},
                                          {"type", type},
                                          {"arrayCount", size / kDescriptorSize},
                                          {"name", name}};
                if (type == "sampledTexture" && space != 0) {
                    const auto annotation = FindAnnotation(defaults, name);
                    if (annotation != defaults.end() &&
                        !BuildTextureDefault(*annotation, binding["default"], error)) {
                        return false;
                    }
                }
                if (type == "uniformBuffer") {
                    const auto block = std::find_if(
                        blocks.begin(), blocks.end(),
                        [&name](const DxcBufferBlock &item) { return item.name == name; });
                    nlohmann::json members;
                    if (block != blocks.end()) {
                        if (!BuildMembers(*block, space, defaults, members, error)) {
                            return false;
                        }
                        if (!members.empty()) {
                            binding["size"] = block->size;
                            binding["members"] = std::move(members);
                        }
                    } else if (space != 0) {
                        error = "No buffer layout found for " + name + ".";
                        return false;
                    }
                }
                portable["bindings"].push_back(std::move(binding));
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
