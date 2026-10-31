#pragma once

#include <filesystem>
#include <string>

namespace ShaderCompiler {

bool ConvertDxilToMetal(const std::filesystem::path &dxilPath, const std::string &entryPoint,
                        const std::string &stage, const std::filesystem::path &metallibPath,
                        const std::filesystem::path &reflectionPath, bool dryRun,
                        std::string &error);

} // namespace ShaderCompiler
