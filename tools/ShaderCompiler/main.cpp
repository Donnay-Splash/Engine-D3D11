// Shader build tool: HLSL -> DXIL (dxc) -> metallib (Metal Shader Converter library).

#include "ShaderCompilerConfig.h"
#include "MetalShaderConverter.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
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

// Parses a Makefile-style dependency file ("target: dep dep ...") written by dxc -MF.
std::vector<fs::path> ReadDependencies(const fs::path &depFile)
{
    std::ifstream stream(depFile);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    const std::string text = buffer.str();

    std::vector<fs::path> dependencies;
    std::string current;
    bool afterTarget = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '\\' && i + 1 < text.size()) {
            const char next = text[i + 1];
            if (next == '\n' || next == '\r') {
                continue;
            }
            current += next;
            ++i;
        } else if (c == ':' && !afterTarget && (i + 1 == text.size() || text[i + 1] == ' ')) {
            afterTarget = true;
            current.clear();
        } else if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            if (afterTarget && !current.empty()) {
                dependencies.emplace_back(current);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    if (afterTarget && !current.empty()) {
        dependencies.emplace_back(current);
    }
    return dependencies;
}

// True when every output exists and is newer than the source, the config and all headers
// recorded in the dependency file by the previous build. Missing data means "out of date".
bool IsUpToDate(const std::vector<fs::path> &outputs, const fs::path &depFile,
                const std::vector<fs::path> &extraInputs)
{
    std::error_code ec;
    fs::file_time_type oldestOutput = fs::file_time_type::max();
    for (const fs::path &output : outputs) {
        const auto time = fs::last_write_time(output, ec);
        if (ec) {
            return false;
        }
        oldestOutput = std::min(oldestOutput, time);
    }

    std::vector<fs::path> inputs = extraInputs;
    if (!fs::exists(depFile, ec)) {
        return false;
    }
    const auto dependencies = ReadDependencies(depFile);
    if (dependencies.empty()) {
        return false;
    }
    inputs.insert(inputs.end(), dependencies.begin(), dependencies.end());

    for (const fs::path &input : inputs) {
        const auto time = fs::last_write_time(input, ec);
        if (ec || time > oldestOutput) {
            return false;
        }
    }
    return true;
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
            const fs::path depPath = dxilDirectory / (baseName + ".d");
            const fs::path metallibPath = metalDirectory / (baseName + ".metallib");
            const fs::path reflectionPath = metalDirectory / (baseName + ".json");

            std::vector<fs::path> outputs = {dxilPath};
            if (buildMetal) {
                outputs.push_back(metallibPath);
                outputs.push_back(reflectionPath);
            }
            std::vector<fs::path> extraInputs = {shader.source, configPath};
            if (config->dxc.find('/') != std::string::npos) {
                extraInputs.emplace_back(config->dxc);
            }
            if (!dryRun && IsUpToDate(outputs, depPath, extraInputs)) {
                std::cout << baseName << " is up to date\n";
                continue;
            }
            std::filesystem::remove(depPath);

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

            // Same arguments with -MF and no -Fo: dxc then only records the header dependencies.
            std::vector<std::string> dependencyArguments = {dxc.front()};
            for (std::size_t i = 1; i < dxc.size(); ++i) {
                if (dxc[i] == "-Fo") {
                    ++i;
                    continue;
                }
                dependencyArguments.push_back(dxc[i]);
            }
            dependencyArguments.insert(dependencyArguments.begin() + 1, {"-MF", depPath.string()});

            if (!Run(dxc, dryRun) || !Run(dependencyArguments, dryRun)) {
                std::cerr << "dxc failed for " << baseName << '\n';
                return 1;
            }

            if (!buildMetal) {
                continue;
            }
            std::string conversionError;
            if (!ShaderCompiler::ConvertDxilToMetal(
                    dxilPath, entry.name, entry.stage, metallibPath, reflectionPath, dryRun, conversionError)) {
                std::cerr << conversionError << '\n';
                return 1;
            }
        }
    }
    return 0;
}
