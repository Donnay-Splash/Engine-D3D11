#include "DxcBufferLayout.h"

#include <regex>
#include <sstream>

namespace ShaderCompiler {

std::vector<DxcBufferBlock> ParseDxcBufferBlocks(const std::string &disassembly)
{
    static const std::regex blockStart(R"(^;\s*cbuffer\s+(\w+)\s*$)");
    static const std::regex member(R"(^;\s+([\w<>]+)\s+(\w+(?:\[\d+\])?);\s+;\s*Offset:\s*(\d+)\s*$)");
    static const std::regex structEnd(R"(^;\s+\}\s+\w+;\s+;\s*Offset:\s*\d+\s+Size:\s*(\d+)\s*$)");

    std::vector<DxcBufferBlock> blocks;
    std::istringstream stream(disassembly);
    std::string line;
    bool inBlock = false;
    std::smatch match;
    while (std::getline(stream, line)) {
        if (std::regex_match(line, match, blockStart)) {
            blocks.push_back({match[1].str(), 0, {}});
            inBlock = true;
        } else if (inBlock && std::regex_match(line, match, member)) {
            blocks.back().members.push_back(
                {match[2].str(), match[1].str(), static_cast<unsigned>(std::stoul(match[3]))});
        } else if (inBlock && std::regex_match(line, match, structEnd)) {
            blocks.back().size = static_cast<unsigned>(std::stoul(match[1]));
            inBlock = false;
        }
    }
    return blocks;
}

std::vector<DxcResourceBinding> ParseDxcResourceBindings(const std::string &disassembly)
{
    static const std::regex bind(R"(^[a-z]+(\d+)(?:,space(\d+))?$)");

    std::vector<DxcResourceBinding> resources;
    std::istringstream stream(disassembly);
    std::string line;
    bool inTable = false;
    while (std::getline(stream, line)) {
        if (line.rfind("; Resource Bindings:", 0) == 0) {
            inTable = true;
            continue;
        }
        if (!inTable || line.size() < 2 || line == ";") {
            continue;
        }
        if (line.rfind("; Name", 0) == 0 || line.rfind("; ---", 0) == 0) {
            continue;
        }
        std::istringstream fields(line.substr(1));
        std::vector<std::string> tokens;
        for (std::string token; fields >> token;) {
            tokens.push_back(token);
        }
        std::smatch match;
        if (tokens.size() < 3 || !std::regex_match(tokens[tokens.size() - 2], match, bind)) {
            break;
        }
        resources.push_back({tokens[0], tokens[1], static_cast<unsigned>(std::stoul(match[1])),
                             match[2].matched ? static_cast<unsigned>(std::stoul(match[2])) : 0});
    }
    return resources;
}

} // namespace ShaderCompiler
