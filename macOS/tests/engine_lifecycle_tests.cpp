#include "Engine.h"
#include "EngineBackend.h"

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

class FakeSurface final : public engine::Surface
{
public:
    explicit FakeSurface(std::vector<std::string> &events, bool canStart = true)
        : events(events), canStart(canStart)
    {
    }

    void *nativeHandle() noexcept override { return this; }
    engine::Size drawableSize() const noexcept override { return {800, 600}; }

    bool start(const Callbacks &callbacks, engine::Error &error) override
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

    void stop() noexcept override { events.push_back("surface.stop"); }

    bool draw(engine::Error &error)
    {
        return callbacks_.draw(callbacks_.context, this, error);
    }

    bool resize(engine::Size size, engine::Error &error)
    {
        return callbacks_.resize(callbacks_.context, size, error);
    }

    void fail(const engine::Error &error)
    {
        callbacks_.failure(callbacks_.context, error);
    }

    std::vector<std::string> &events;
    bool canStart;

private:
    Callbacks callbacks_{};
};

class FakeRenderer final : public engine::detail::Renderer
{
public:
    explicit FakeRenderer(std::vector<std::string> &events, bool canInitialize = true)
        : events(events), canInitialize(canInitialize)
    {
    }

    bool initialize(
        void *surface,
        engine::Size size,
        const engine::ErrorHandler &errorHandler,
        engine::Error &error) override
    {
        errorHandler_ = errorHandler;
        (void)error;
        check(surface != nullptr);
        check(size.width == 800 && size.height == 600);
        events.push_back("renderer.initialize");
        if (!canInitialize) {
            error.message = "renderer initialization failed";
            return false;
        }
        return true;
    }

    bool render(void *frame, engine::Error &error) override
    {
        (void)error;
        check(frame != nullptr);
        events.push_back("renderer.render");
        return true;
    }

    bool resize(engine::Size size, engine::Error &error) override
    {
        (void)error;
        check(size.width == 1024 && size.height == 768);
        events.push_back("renderer.resize");
        return true;
    }

    void shutdown() noexcept override { events.push_back("renderer.shutdown"); }

    void emitFailure(const engine::Error &error)
    {
        if (errorHandler_) {
            errorHandler_(error);
        }
    }

    std::vector<std::string> &events;
    bool canInitialize;

private:
    engine::ErrorHandler errorHandler_;
};

void hostCanAttachRenderResizeAndShutDownInOrder()
{
    std::vector<std::string> events;
    std::unique_ptr<FakeRenderer> renderer(new FakeRenderer(events));
    std::unique_ptr<engine::Engine> instance =
        engine::detail::EngineFactory::create(std::move(renderer));
    FakeSurface surface(events);
    engine::Error error;

    check(instance->initialize(surface, [](const engine::Error &) {}, error));
    check(surface.draw(error));
    check(surface.resize({1024, 768}, error));
    instance->shutdown();

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
    std::unique_ptr<engine::Engine> instance =
        engine::detail::EngineFactory::create(std::move(renderer));
    FakeSurface surface(events, false);
    engine::Error error;

    check(!instance->initialize(surface, [](const engine::Error &) {}, error));
    check(error.message == "surface startup failed");
    instance->shutdown();
    check(events == std::vector<std::string>({
        "renderer.initialize", "surface.start", "surface.stop", "renderer.shutdown"
    }));
}

void rendererInitializationFailureIsVisibleAndDoesNotStartSurface()
{
    std::vector<std::string> events;
    std::unique_ptr<engine::Engine> instance = engine::detail::EngineFactory::create(
        std::unique_ptr<FakeRenderer>(new FakeRenderer(events, false)));
    FakeSurface surface(events);
    engine::Error error;

    check(!instance->initialize(surface, [](const engine::Error &) {}, error));
    check(error.message == "renderer initialization failed");
    instance->shutdown();
    check(events == std::vector<std::string>({
        "renderer.initialize", "renderer.shutdown"
    }));
}

void runtimeSurfaceFailureReachesHostOnlyOnce()
{
    std::vector<std::string> events;
    std::unique_ptr<FakeRenderer> renderer(new FakeRenderer(events));
    FakeRenderer *rendererEvents = renderer.get();
    std::unique_ptr<engine::Engine> instance =
        engine::detail::EngineFactory::create(std::move(renderer));
    FakeSurface surface(events);
    engine::Error error;
    int reports = 0;
    std::string reportedMessage;

    check(instance->initialize(surface, [&reports, &reportedMessage](const engine::Error &runtimeError) {
        ++reports;
        reportedMessage = runtimeError.message;
    }, error));
    surface.fail({"surface was lost"});
    check(reports == 1);
    check(reportedMessage == "surface was lost");
    rendererEvents->emitFailure({"late renderer failure"});
    check(reports == 1);
    check(!surface.draw(error));
    check(reports == 1);
    instance->shutdown();
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
