#import "MetalRenderer.h"

namespace engine {

std::unique_ptr<Engine> createEngine()
{
    return detail::EngineFactory::create(createMetalRenderer());
}

} // namespace engine
