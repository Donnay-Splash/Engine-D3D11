#pragma once

#include "ShaderLibrary.h"

#include <cstdint>
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
    bool Load(Error &error);

    std::vector<std::uint8_t> m_ShaderBytecode;
};

} // namespace Metal
} // namespace Engine
