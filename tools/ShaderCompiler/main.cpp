// Command line shader build tool: HLSL -> DXIL (dxc) -> metallib (metal-shaderconverter).

#include "ShaderCompilerConfig.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using ShaderCompiler::Config;

namespace {

void PrintUsage()
{
    std::cerr << "Usage: shader-compiler --config <file> [--target dxil|metal|all] [--dry-run]\n"
                 "  --target   dxil: HLSL to DXIL only; metal: DXIL to metallib (also runs dxil);\n"
                 "             all (default) is the same as metal.\n"
                 "  --dry-run  print the commands without running them\n";
}

std::string Quote(const std::string &argument)
{
    return "\"" + argument + "\"";
}

bool Run(const std::vector<std::string> &arguments, bool dryRun)
{
    std::string command;
    for (const std::string &argument : arguments) {
        if (!argument.empty()) {
            command += (command.empty() ? "" : " ") + Quote(argument);
        }
    }
    std::cout << command << '\n';
    return dryRun || std::system(command.c_str()) == 0;
}

std::string ProfileFor(const std::string &stage, const std::string &model)
{
    return (stage == "vs" ? "vs_" : stage == "ps" ? "ps_" : "cs_") + model;
}

} // namespace

int main(int argc, char **argv)
{
    fs::path configPath;
    bool buildMetal = true;
    bool dryRun = false;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        } else if (argument == "--target" && i + 1 < argc) {
            const std::string target = argv[++i];
            if (target != "dxil" && target != "metal" && target != "all") {
                PrintUsage();
                return 2;
            }
            buildMetal = target != "dxil";
        } else if (argument == "--dry-run") {
            dryRun = true;
        } else {
            PrintUsage();
            return 2;
        }
    }
    if (configPath.empty()) {
        PrintUsage();
        return 2;
    }

    std::string error;
    const auto config = ShaderCompiler::LoadConfig(configPath, error);
    if (!config) {
        std::cerr << error << '\n';
        return 1;
    }

    const fs::path dxilDirectory = config->outputDirectory / "dxil";
    const fs::path metalDirectory = config->outputDirectory / "metal";
    if (!dryRun) {
        fs::create_directories(dxilDirectory);
        if (buildMetal) {
            fs::create_directories(metalDirectory);
        }
    }

    for (const auto &shader : config->shaders) {
        for (const auto &entry : shader.entries) {
            const std::string baseName = shader.name + "_" + entry.name;
            const fs::path dxilPath = dxilDirectory / (baseName + ".dxil");

            std::vector<std::string> dxc = {
                config->dxc,
                "-T", ProfileFor(entry.stage, config->shaderModel),
                "-E", entry.name,
                "-Fo", dxilPath.string(),
            };
            if (!config->includeDirectory.empty()) {
                dxc.push_back("-I");
                dxc.push_back(config->includeDirectory.string());
            }
            dxc.insert(dxc.end(), config->dxcArgs.begin(), config->dxcArgs.end());
            dxc.push_back(shader.source.string());
            if (!Run(dxc, dryRun)) {
                std::cerr << "dxc failed for " << baseName << '\n';
                return 1;
            }

            if (!buildMetal) {
                continue;
            }
            std::vector<std::string> converter = {
                config->metalShaderConverter,
                dxilPath.string(),
                "-o", (metalDirectory / (baseName + ".metallib")).string(),
                "--output-reflection-file",
                (metalDirectory / (baseName + ".json")).string(),
            };
            converter.insert(converter.end(), config->metalArgs.begin(), config->metalArgs.end());
            converter.insert(converter.end(), entry.metalArgs.begin(), entry.metalArgs.end());
            if (!Run(converter, dryRun)) {
                std::cerr << "metal-shaderconverter failed for " << baseName << '\n';
                return 1;
            }
        }
    }
    return 0;
}
