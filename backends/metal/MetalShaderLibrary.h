#pragma once

#include "ShaderLibrary.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Engine {
namespace Metal {

class MetalShaderLibrary final : public ShaderLibrary
{
public:
    bool LoadProgramConfig(std::string &text, Error &error) override;

    bool RequestShader(
        Graphics::Device &device,
        const ShaderRef &ref,
        Graphics::ShaderStage stage,
        LoadedShader &loaded,
        Error &error) override;

private:
    // Reads <bundle resources>/shaders/<fileName>.
    static bool Load(const std::string &fileName, std::vector<std::uint8_t> &data, Error &error);
};

} // namespace Metal
} // namespace Engine
