#include "Engine.h"
#include "Device.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool condition)
{
    if (!condition) {
        throw std::runtime_error("Engine lifecycle assertion failed.");
    }
}

class FakeSurface final : public Engine::Surface
{
public:
    explicit FakeSurface(std::vector<std::string> &events, bool canStart = true)
        : events(events), canStart(canStart)
    {
    }

    void *NativeHandle() noexcept override { return this; }
    Engine::Size DrawableSize() const noexcept override { return {800, 600}; }

    bool Start(const Callbacks &callbacks, Engine::Error &error) override
    {
        (void)error;
        events.push_back("surface.start");
        callbacks_ = callbacks;
        if (!canStart) {
            error.message = "surface startup failed";
            return false;
        }
        return true;
    }

    void Stop() noexcept override { events.push_back("surface.stop"); }

    bool Draw(Engine::Error &error)
    {
        return callbacks_.Draw(callbacks_.context, this, error);
    }

    bool Resize(Engine::Size size, Engine::Error &error)
    {
        return callbacks_.Resize(callbacks_.context, size, error);
    }

    void fail(const Engine::Error &error)
    {
        callbacks_.Failure(callbacks_.context, error);
    }

    std::vector<std::string> &events;
    bool canStart;

private:
    Callbacks callbacks_{};
};

class FakeRenderer final : public Engine::Device
{
public:
    explicit FakeRenderer(std::vector<std::string> &events, bool canInitialize = true)
        : events(events), canInitialize(canInitialize)
    {
    }

    bool Initialize(
        void *surface,
        Engine::Size size,
        const Engine::ErrorHandler &errorHandler,
        Engine::Error &error) override
    {
        errorHandler_ = errorHandler;
        (void)error;
        std::string sceneError;
        check(surface != nullptr);
        check(size.width == 800 && size.height == 600);
        events.push_back("renderer.initialize");
        if (!canInitialize) {
            error.message = "renderer initialization failed";
            return false;
        }
        return true;
    }

    bool Resize(Engine::Size size, Engine::Error &error) override
    {
        (void)error;
        check(size.width == 1024 && size.height == 768);
        events.push_back("renderer.resize");
        return true;
    }

    void Shutdown() noexcept override { events.push_back("renderer.shutdown"); }

    void emitFailure(const Engine::Error &error)
    {
        if (errorHandler_) {
            errorHandler_(error);
        }
    }

    std::vector<std::string> &events;
    bool canInitialize;

private:
    Engine::ErrorHandler errorHandler_;
};

void hostCanAttachRenderResizeAndShutDownInOrder()
{
    std::vector<std::string> events;
    std::unique_ptr<FakeRenderer> renderer(new FakeRenderer(events));
    std::unique_ptr<Engine::Engine> instance =
        Engine::EngineFactory::create(std::move(renderer));
    FakeSurface surface(events);
    Engine::Error error;

    check(instance->Initialize(surface, [](const Engine::Error &) {}, error));
    check(surface.Draw(error));
    check(surface.Resize({1024, 768}, error));
    instance->Shutdown();

    const std::vector<std::string> expected = {
        "renderer.initialize",
        "surface.start",
        "renderer.render",
        "renderer.resize",
        "surface.stop",
        "renderer.shutdown",
    };
    check(events == expected);
}

void startupFailureIsVisibleAndCleansUpInOrder()
{
    std::vector<std::string> events;
    std::unique_ptr<FakeRenderer> renderer(new FakeRenderer(events));
    std::unique_ptr<Engine::Engine> instance =
        Engine::EngineFactory::create(std::move(renderer));
    FakeSurface surface(events, false);
    Engine::Error error;

    check(!instance->Initialize(surface, [](const Engine::Error &) {}, error));
    check(error.message == "surface startup failed");
    instance->Shutdown();
    check(events == std::vector<std::string>({
        "renderer.initialize", "surface.start", "surface.stop", "renderer.shutdown"
    }));
}

void rendererInitializationFailureIsVisibleAndDoesNotStartSurface()
{
    std::vector<std::string> events;
    std::unique_ptr<Engine::Engine> instance = Engine::EngineFactory::create(
        std::unique_ptr<FakeRenderer>(new FakeRenderer(events, false)));
    FakeSurface surface(events);
    Engine::Error error;

    check(!instance->Initialize(surface, [](const Engine::Error &) {}, error));
    check(error.message == "renderer initialization failed");
    instance->Shutdown();
    check(events == std::vector<std::string>({
        "renderer.initialize", "renderer.shutdown"
    }));
}

void runtimeSurfaceFailureReachesHostOnlyOnce()
{
    std::vector<std::string> events;
    std::unique_ptr<FakeRenderer> renderer(new FakeRenderer(events));
    FakeRenderer *rendererEvents = renderer.get();
    std::unique_ptr<Engine::Engine> instance =
        Engine::EngineFactory::create(std::move(renderer));
    FakeSurface surface(events);
    Engine::Error error;
    int reports = 0;
    std::string reportedMessage;

    check(instance->Initialize(surface, [&reports, &reportedMessage](const Engine::Error &runtimeError) {
        ++reports;
        reportedMessage = runtimeError.message;
    }, error));
    surface.fail({"surface was lost"});
    check(reports == 1);
    check(reportedMessage == "surface was lost");
    rendererEvents->emitFailure({"late renderer failure"});
    check(reports == 1);
    check(!surface.Draw(error));
    check(reports == 1);
    instance->Shutdown();
    rendererEvents->emitFailure({"failure after shutdown"});
    check(reports == 1);
    check(events == std::vector<std::string>({
        "renderer.initialize", "surface.start", "surface.stop", "renderer.shutdown"
    }));
}

} // namespace

int main()
{
    hostCanAttachRenderResizeAndShutDownInOrder();
    startupFailureIsVisibleAndCleansUpInOrder();
    rendererInitializationFailureIsVisibleAndDoesNotStartSurface();
    runtimeSurfaceFailureReachesHostOnlyOnce();
}
