#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

namespace Engine {
namespace Graphics {

class Shader : public Resource
{
public:
    virtual ShaderStage Stage() const noexcept = 0;
    virtual const std::string &EntryPoint() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
