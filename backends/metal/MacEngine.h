#pragma once

#include "Graphics/Adapter.h"
#include "Graphics/GraphicsInstance.h"

#include <memory>

namespace Engine {
class Engine;
namespace Metal {

std::shared_ptr<Graphics::GraphicsInstance> CreateGraphicsInstance();
std::unique_ptr<Engine> CreateEngine(
    std::shared_ptr<Graphics::Adapter> preferredAdapter = {});

} // namespace Metal    
} // namespace Engine