#pragma once

#include "Graphics/GraphicsInstance.h"

#include <memory>

namespace Engine {
namespace Metal {

std::shared_ptr<Graphics::GraphicsInstance> CreateMetalGraphicsInstance();

} // namespace Metal
} // namespace Engine
