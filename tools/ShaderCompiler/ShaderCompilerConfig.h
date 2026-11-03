#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ShaderCompiler {

struct Config
{
    std::string dxc = "dxc";
    std::string shaderModel = "6_6";
    std::filesystem::path outputDirectory;
    std::filesystem::path includeDirectory;
    // Engine program config (ShaderPrograms.cfg); it decides which entries get built and is
    // copied to the output.
    std::filesystem::path programsFile;
    std::vector<std::string> dxcArgs;
};

// Parses the config file; on failure returns nullopt and fills error.
std::optional<Config> LoadConfig(const std::filesystem::path &path, std::string &error);

} // namespace ShaderCompiler
