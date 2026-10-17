#ifndef ENGINE_MACOS_METAL_RENDERER_H
#define ENGINE_MACOS_METAL_RENDERER_H

#include "Engine.h"
#include "EngineBackend.h"

#include <memory>

namespace engine {

std::unique_ptr<detail::Renderer> createMetalRenderer();

} // namespace engine

#endif
