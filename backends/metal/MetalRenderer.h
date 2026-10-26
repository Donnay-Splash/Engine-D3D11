#pragma once
#include "Engine.h"
#include "Renderer.h"

#include <memory>

namespace Engine {

std::unique_ptr<Renderer> CreateMetalRenderer();

} // namespace Engine
