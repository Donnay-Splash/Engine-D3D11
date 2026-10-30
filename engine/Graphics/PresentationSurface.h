#pragma once

#include "GraphicsTypes.h"

namespace Engine {
namespace Graphics {

class PresentationSurface
{
public:
    virtual ~PresentationSurface() = default;
    virtual Extent3D DrawableExtent() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
