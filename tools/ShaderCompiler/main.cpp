// Shader build tool: HLSL -> DXIL (dxc) -> metallib (Metal Shader Converter library).

#include "ShaderCompilerConfig.h"
#include "MetalShaderConverter.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
using ShaderCompiler::Config;

extern char **environ;

namespace {

void PrintUsage()
{
    std::cerr << "Usage: shader-compiler --config <file> [--target dxil|metal|all] [--dry-run]\n"
                 "  --target   dxil: HLSL to DXIL only; metal: DXIL to metallib (also runs dxil);\n"
                 "             all (default) is the same as metal.\n"
                 "  --dry-run  print the commands without running them\n";
}

bool Run(const std::vector<std::string> &arguments, bool dryRun)
{
    if (arguments.empty()) {
        return false;
    }

    for (std::size_t index = 0; index < arguments.size(); ++index) {
        std::cout << (index == 0 ? "" : " ") << std::quoted(arguments[index]);
    }
    std::cout << '\n';
    if (dryRun) {
        return true;
    }

    std::vector<char *> argv;
    argv.reserve(arguments.size() + 1);
    for (const std::string &argument : arguments) {
        argv.push_back(const_cast<char *>(argument.c_str()));
    }
    argv.push_back(nullptr);

    pid_t process = 0;
    const int spawnError =
        posix_spawnp(&process, arguments.front().c_str(), nullptr, nullptr, argv.data(), environ);
    if (spawnError != 0) {
        std::cerr << "Failed to start " << std::quoted(arguments.front()) << ": "
                  << std::strerror(spawnError) << '\n';
        return false;
    }

    int status = 0;
    pid_t waited = 0;
    do {
        waited = waitpid(process, &status, 0);
    } while (waited == -1 && errno == EINTR);
    return waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0;
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
            std::string conversionError;
            if (!ShaderCompiler::ConvertDxilToMetal(
                    dxilPath, entry.name, entry.stage, metalDirectory / (baseName + ".metallib"),
                    metalDirectory / (baseName + ".json"), dryRun, conversionError)) {
                std::cerr << conversionError << '\n';
                return 1;
            }
        }
    }
    return 0;
}
