#include "Engine.h"
#include "EngineFactory.h"

#include <stdexcept>

namespace {

void check(bool condition)
{
    if (!condition) {
        throw std::runtime_error("Engine lifecycle assertion failed.");
    }
}

class NeverStartedSurface final : public Engine::Surface
{
public:
    Engine::Size DrawableSize() const noexcept override { return {800, 600}; }

    bool Start(const Callbacks &, Engine::Error &) override
    {
        started = true;
        return true;
    }

    void Stop() noexcept override {}

    bool started = false;
};

void missingGraphicsServicesFailBeforeSurfaceStartup()
{
    std::unique_ptr<Engine::Engine> engine = Engine::EngineFactory::create(nullptr, nullptr);
    NeverStartedSurface surface;
    Engine::Error error;

    check(!engine->Initialize(surface, [](const Engine::Error &) {}, error));
    check(error.message == "The engine requires a graphics instance and shader library.");
    check(!surface.started);
}

} // namespace

int main()
{
    missingGraphicsServicesFailBeforeSurfaceStartup();
}
