#ifndef ENGINE_MACOS_METAL_SURFACE_ADAPTER_H
#define ENGINE_MACOS_METAL_SURFACE_ADAPTER_H

#import <MetalKit/MetalKit.h>

#include "Engine.h"

namespace Engine {
namespace Metal {

std::unique_ptr<Surface> CreateMetalSurface(MTKView *view);

} // namespace Metal
} // namespace Engine

#endif
