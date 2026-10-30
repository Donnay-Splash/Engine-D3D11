#pragma once

#include "Engine.h"

#include <memory>

namespace CA {
class MetalLayer;
}

namespace Engine {
namespace Metal {

class MetalSurfaceAdapter : public Surface
{
public:
    virtual CA::MetalLayer *Layer() const noexcept = 0;
};

std::unique_ptr<Surface> CreateMetalSurface(CA::MetalLayer *layer);
bool DispatchMetalDraw(Surface &surface, Error &error);
bool DispatchMetalResize(Surface &surface, Size size, Error &error);

} // namespace Metal
} // namespace Engine
