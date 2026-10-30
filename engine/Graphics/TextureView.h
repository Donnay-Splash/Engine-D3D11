#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"
#include "Texture.h"

#include <memory>

namespace Engine {
namespace Graphics {

class TextureView : public Resource
{
public:
    virtual std::shared_ptr<Texture> GetTexture() const = 0;
    virtual const ResourceRange &Range() const noexcept = 0;
    virtual TextureFormat Format() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
