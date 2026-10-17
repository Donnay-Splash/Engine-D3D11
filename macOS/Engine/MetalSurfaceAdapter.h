#ifndef ENGINE_MACOS_METAL_SURFACE_ADAPTER_H
#define ENGINE_MACOS_METAL_SURFACE_ADAPTER_H

#import <MetalKit/MetalKit.h>

#include "Engine.h"

namespace engine {
namespace macos {

std::unique_ptr<Surface> createMetalSurface(MTKView *view);

} // namespace macos
} // namespace engine

#endif
