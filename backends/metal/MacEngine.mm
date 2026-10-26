#include "EngineFactory.h"
#include "Engine.h"

namespace Engine {
namespace Metal {
std::unique_ptr<Engine> CreateEngine()
{
    return EngineFactory::create(nullptr, nullptr);
}

} // namespace Metal
} // namespace Engine
