#import "MetalDevice.h"

namespace Engine {
namespace Metal {
std::unique_ptr<Engine> CreateEngine()
{
    return EngineFactory::create(CreateMetalDevice());
}

} // namespace Metal
} // namespace Engine
