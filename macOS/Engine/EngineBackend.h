#ifndef ENGINE_MACOS_ENGINE_BACKEND_H
#define ENGINE_MACOS_ENGINE_BACKEND_H

#include "Engine.h"

namespace engine {
namespace detail {

class Renderer
{
public:
    virtual ~Renderer() {}
    virtual bool initialize(
        void *surface,
        Size size,
        const ErrorHandler &errorHandler,
        Error &error) = 0;
    virtual bool render(void *frame, Error &error) = 0;
    virtual bool resize(Size size, Error &error) = 0;
    virtual void shutdown() noexcept = 0;
};

class EngineFactory
{
public:
    static std::unique_ptr<Engine> create(std::unique_ptr<Renderer> renderer);
};

} // namespace detail
} // namespace engine

#endif
