#include "ShaderCompilerConfig.h"

#include <fstream>
#include <sstream>

namespace ShaderCompiler {
namespace {

std::string Trim(const std::string &text)
{
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    return text.substr(begin, text.find_last_not_of(" \t\r\n") - begin + 1);
}

std::vector<std::string> Split(const std::string &text)
{
    std::istringstream stream(text);
    std::vector<std::string> tokens;
    for (std::string token; stream >> token;) {
        tokens.push_back(token);
    }
    return tokens;
}

} // namespace

std::optional<Config> LoadConfig(const std::filesystem::path &path, std::string &error)
{
    std::ifstream file(path);
    if (!file) {
        error = "Cannot open config file: " + path.string();
        return std::nullopt;
    }

    const std::filesystem::path baseDirectory = path.parent_path();
    const auto resolve = [&](const std::string &value) {
        return (baseDirectory / value).lexically_normal();
    };

    Config config;
    enum class Section { None, Settings, Shader } section = Section::None;
    std::string line;
    int lineNumber = 0;
    const auto fail = [&](const std::string &message) {
        error = path.string() + ":" + std::to_string(lineNumber) + ": " + message;
        return std::nullopt;
    };

    while (std::getline(file, line)) {
        ++lineNumber;
        const auto comment = line.find('#');
        if (comment != std::string::npos) {
            line.erase(comment);
        }
        line = Trim(line);
        if (line.empty()) {
            continue;
        }
        if (line == "[settings]") {
            section = Section::Settings;
            continue;
        }
        if (line == "[shader]") {
            section = Section::Shader;
            config.shaders.emplace_back();
            continue;
        }

        const auto equals = line.find('=');
        if (equals == std::string::npos || section == Section::None) {
            return fail("expected a section header or 'key = value'");
        }
        const std::string key = Trim(line.substr(0, equals));
        const std::string value = Trim(line.substr(equals + 1));

        if (section == Section::Settings) {
            if (key == "dxc") {
                // A bare command name is looked up on PATH; anything with a directory is
                // relative to the config file.
                config.dxc = value.find('/') == std::string::npos
                                 ? value
                                 : resolve(value).string();
            } else if (key == "shaderModel") {
                config.shaderModel = value;
            } else if (key == "outputDirectory") {
                config.outputDirectory = resolve(value);
            } else if (key == "includeDirectory") {
                config.includeDirectory = resolve(value);
            } else if (key == "dxcArgs") {
                config.dxcArgs = Split(value);
            } else {
                return fail("unknown setting '" + key + "'");
            }
            continue;
        }

        ShaderSource &shader = config.shaders.back();
        if (key == "name") {
            shader.name = value;
        } else if (key == "source") {
            shader.source = resolve(value);
        } else if (key == "entry") {
            std::vector<std::string> tokens = Split(value);
            if (tokens.size() != 2 ||
                (tokens[0] != "vs" && tokens[0] != "ps" && tokens[0] != "cs")) {
                return fail("entry must be '<vs|ps|cs> <entryPoint>'");
            }
            EntryPoint entry;
            entry.stage = tokens[0];
            entry.name = tokens[1];
            shader.entries.push_back(std::move(entry));
        } else {
            return fail("unknown shader key '" + key + "'");
        }
    }

    if (config.outputDirectory.empty()) {
        error = "Config is missing settings.outputDirectory";
        return std::nullopt;
    }
    for (const ShaderSource &shader : config.shaders) {
        if (shader.name.empty() || shader.source.empty() || shader.entries.empty()) {
            error = "Every [shader] needs a name, a source and at least one entry";
            return std::nullopt;
        }
    }
    return config;
}

} // namespace ShaderCompiler
