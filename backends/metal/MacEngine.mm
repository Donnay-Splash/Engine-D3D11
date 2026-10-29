#include "EngineFactory.h"
#include "Engine.h"
#include "MetalShaderLibrary.h"

#include <memory>

namespace Engine {
namespace Metal {
std::unique_ptr<Engine> CreateEngine()
{
    return EngineFactory::create(
        nullptr,
        std::make_shared<MetalShaderLibrary>());
}

} // namespace Metal
} // namespace Engine
