#pragma once
#include <memory>
namespace Engine {
    class Engine;
namespace Metal {
std::unique_ptr<Engine> CreateEngine();
} // namespace Metal    
} // namespace Engine