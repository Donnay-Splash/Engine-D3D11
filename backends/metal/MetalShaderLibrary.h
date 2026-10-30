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
    bool RequestGraphicsShaders(
        Graphics::Device &device,
        ShaderProgramType type,
        const std::string &name,
        GraphicsShaderPair &shaders,
        Error &error) override;

private:
    static bool Load(const std::string &entryPoint, const std::string &extension,
                     std::vector<std::uint8_t> &data, Error &error);
};

} // namespace Metal
} // namespace Engine
