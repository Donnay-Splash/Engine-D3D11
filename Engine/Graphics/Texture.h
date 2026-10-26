#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

namespace Engine {
namespace Graphics {

class Texture : public Resource
{
public:
    virtual const TextureDesc &Description() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
