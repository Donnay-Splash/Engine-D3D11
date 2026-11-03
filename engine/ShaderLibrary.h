#pragma once

#include "Graphics/Shader.h"
#include "Graphics/ShaderReflection.h"
#include "ShaderProgramCatalog.h"

#include <memory>
#include <string>

namespace Engine {

struct Error;
namespace Graphics {
class Device;
}

struct LoadedShader
{
    std::shared_ptr<Graphics::Shader> shader;
    Graphics::ShaderReflection reflection;
};

// Supplies compiled shader artifacts. The backend owns where they live and what format
// they take; the engine only names them by source and entry point.
class ShaderLibrary
{
public:
    virtual ~ShaderLibrary() = default;

    // Returns the text of ShaderPrograms.cfg.
    virtual bool LoadProgramConfig(std::string &text, Error &error) = 0;

    virtual bool RequestShader(
        Graphics::Device &device,
        const ShaderRef &ref,
        Graphics::ShaderStage stage,
        LoadedShader &loaded,
        Error &error) = 0;
};

} // namespace Engine
