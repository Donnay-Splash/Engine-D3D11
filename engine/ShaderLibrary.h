#pragma once

#include "Graphics/Shader.h"

#include <memory>
#include <string>

namespace Engine {

struct Error;
namespace Graphics {
class Device;
}

enum class ShaderProgramType
{
    Mesh
};

struct GraphicsShaderPair
{
    std::shared_ptr<Graphics::Shader> vertex;
    std::shared_ptr<Graphics::Shader> fragment;
};

class ShaderLibrary
{
public:
    virtual ~ShaderLibrary() = default;
    virtual bool RequestGraphicsShaders(
        Graphics::Device &device,
        ShaderProgramType type,
        const std::string &name,
        GraphicsShaderPair &shaders,
        Error &error) = 0;
};

} // namespace Engine
