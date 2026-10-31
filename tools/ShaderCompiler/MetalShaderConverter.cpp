#include "MetalShaderConverter.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <vector>

#ifdef __APPLE__
#include <metal_irconverter/metal_irconverter.h>
#endif

namespace ShaderCompiler {
namespace {

#ifdef __APPLE__
IRShaderStage ToMetalStage(const std::string &stage)
{
    if (stage == "vs") {
        return IRShaderStageVertex;
    }
    if (stage == "ps") {
        return IRShaderStageFragment;
    }
    return IRShaderStageCompute;
}

bool WriteFile(const std::filesystem::path &path, const std::uint8_t *bytes, std::size_t size,
               std::string &error)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        error = "Metal shader output is too large to write: " + path.string();
        return false;
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        error = "Cannot create Metal shader output: " + path.string();
        return false;
    }
    output.write(reinterpret_cast<const char *>(bytes), static_cast<std::streamsize>(size));
    if (!output) {
        error = "Failed to write Metal shader output: " + path.string();
        return false;
    }
    return true;
}
#endif

} // namespace

bool ConvertDxilToMetal(const std::filesystem::path &dxilPath, const std::string &entryPoint,
                        const std::string &stage, const std::filesystem::path &metallibPath,
                        const std::filesystem::path &reflectionPath, bool dryRun,
                        std::string &error)
{
    if (dryRun) {
        std::cout << "libmetalirconverter " << dxilPath << " -> " << metallibPath << " ("
                  << reflectionPath << ")\n";
        return true;
    }

#ifndef __APPLE__
    static_cast<void>(dxilPath);
    static_cast<void>(entryPoint);
    static_cast<void>(stage);
    static_cast<void>(metallibPath);
    static_cast<void>(reflectionPath);
    error = "Metal shader conversion requires macOS and the vendored Apple library.";
    return false;
#else
    std::ifstream input(dxilPath, std::ios::binary);
    if (!input) {
        error = "Cannot open DXIL input: " + dxilPath.string();
        return false;
    }
    const std::vector<std::uint8_t> dxil{std::istreambuf_iterator<char>(input),
                                         std::istreambuf_iterator<char>()};
    if (input.bad() || dxil.empty()) {
        error = "Cannot read DXIL input: " + dxilPath.string();
        return false;
    }

    std::unique_ptr<IRCompiler, decltype(&IRCompilerDestroy)> compiler(IRCompilerCreate(),
                                                                      &IRCompilerDestroy);
    if (!compiler) {
        error = "Metal Shader Converter could not create a compiler.";
        return false;
    }
    IRCompilerSetEntryPointName(compiler.get(), entryPoint.c_str());
    if (stage == "vs") {
        IRCompilerSetStageInGenerationMode(compiler.get(),
                                           IRStageInCodeGenerationModeUseMetalVertexFetch);
    }

    std::unique_ptr<IRObject, decltype(&IRObjectDestroy)> inputObject(
        IRObjectCreateFromDXIL(dxil.data(), dxil.size(), IRBytecodeOwnershipNone),
        &IRObjectDestroy);
    if (!inputObject) {
        error = "Metal Shader Converter could not read DXIL input: " + dxilPath.string();
        return false;
    }

    IRError *rawCompileError = nullptr;
    std::unique_ptr<IRObject, decltype(&IRObjectDestroy)> outputObject(
        IRCompilerAllocCompileAndLink(compiler.get(), nullptr, inputObject.get(),
                                      &rawCompileError),
        &IRObjectDestroy);
    std::unique_ptr<IRError, decltype(&IRErrorDestroy)> compileError(rawCompileError,
                                                                     &IRErrorDestroy);
    if (!outputObject) {
        error = "Metal Shader Converter failed for " + entryPoint;
        if (compileError) {
            error += " (error code " + std::to_string(IRErrorGetCode(compileError.get())) + ")";
        }
        error += ".";
        return false;
    }

    std::unique_ptr<IRMetalLibBinary, decltype(&IRMetalLibBinaryDestroy)> metallib(
        IRMetalLibBinaryCreate(), &IRMetalLibBinaryDestroy);
    if (!metallib || !IRObjectGetMetalLibBinary(outputObject.get(), ToMetalStage(stage),
                                                 metallib.get())) {
        error = "Metal Shader Converter did not produce a metallib for " + entryPoint + ".";
        return false;
    }
    const std::size_t metallibSize = IRMetalLibGetBytecodeSize(metallib.get());
    std::vector<std::uint8_t> metallibBytes(metallibSize);
    if (metallibSize == 0 ||
        IRMetalLibGetBytecode(metallib.get(), metallibBytes.data()) != metallibSize ||
        !WriteFile(metallibPath, metallibBytes.data(), metallibBytes.size(), error)) {
        if (error.empty()) {
            error = "Metal Shader Converter returned invalid metallib data for " + entryPoint +
                    ".";
        }
        return false;
    }

    std::unique_ptr<IRShaderReflection, decltype(&IRShaderReflectionDestroy)> reflection(
        IRShaderReflectionCreate(), &IRShaderReflectionDestroy);
    if (!reflection || !IRObjectGetReflection(outputObject.get(), ToMetalStage(stage),
                                               reflection.get())) {
        error = "Metal Shader Converter did not produce reflection for " + entryPoint + ".";
        return false;
    }
    const char *json = IRShaderReflectionCopyJSONString(reflection.get());
    if (json == nullptr) {
        error = "Metal Shader Converter could not serialize reflection for " + entryPoint + ".";
        return false;
    }
    const std::string reflectionJson(json);
    IRShaderReflectionReleaseString(json);
    return WriteFile(reflectionPath, reinterpret_cast<const std::uint8_t *>(reflectionJson.data()),
                     reflectionJson.size(), error);
#endif
}

} // namespace ShaderCompiler
