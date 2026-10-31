#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ShaderCompiler {

struct EntryPoint
{
    std::string stage; // vs, ps or cs
    std::string name;
};

struct ShaderSource
{
    std::string name;
    std::filesystem::path source;
    std::vector<EntryPoint> entries;
};

struct Config
{
    std::string dxc = "dxc";
    std::string shaderModel = "6_6";
    std::filesystem::path outputDirectory;
    std::filesystem::path includeDirectory;
    std::vector<std::string> dxcArgs;
    std::vector<ShaderSource> shaders;
};

// Parses the config file; on failure returns nullopt and fills error.
std::optional<Config> LoadConfig(const std::filesystem::path &path, std::string &error);

} // namespace ShaderCompiler
