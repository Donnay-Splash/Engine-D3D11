#pragma once

#include <memory>

namespace Engine {

class Engine;
class ShaderLibrary;
namespace Graphics {
class GraphicsInstance;
class Adapter;
}

class EngineFactory
{
public:
    static std::unique_ptr<Engine> create(
        std::shared_ptr<Graphics::GraphicsInstance> graphicsInstance,
        std::shared_ptr<ShaderLibrary> shaderLibrary,
        std::shared_ptr<Graphics::Adapter> preferredAdapter = {});
};

} // namespace Engine
