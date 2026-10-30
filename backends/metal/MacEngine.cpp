#include "EngineFactory.h"
#include "Engine.h"
#include "MetalBackend.h"
#include "MetalShaderLibrary.h"

#include <memory>
#include <utility>

namespace Engine {
namespace Metal {
std::shared_ptr<Graphics::GraphicsInstance> CreateGraphicsInstance()
{
    return CreateMetalGraphicsInstance();
}

std::unique_ptr<Engine> CreateEngine(
    std::shared_ptr<Graphics::Adapter> preferredAdapter)
{
    return EngineFactory::create(
        CreateMetalGraphicsInstance(),
        std::make_shared<MetalShaderLibrary>(),
        std::move(preferredAdapter));
}

} // namespace Metal
} // namespace Engine
