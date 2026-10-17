#ifndef ENGINE_MACOS_ENGINE_H
#define ENGINE_MACOS_ENGINE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace engine {

class Engine;

namespace detail {
class Renderer;
class EngineFactory;
}

struct Error
{
    std::string message;
};

using ErrorHandler = std::function<void(const Error &)>;

struct Size
{
    std::uint32_t width;
    std::uint32_t height;
};

class Surface
{
public:
    struct Callbacks
    {
        void *context;
        bool (*draw)(void *context, void *frame, Error &error);
        bool (*resize)(void *context, Size size, Error &error);
        void (*failure)(void *context, const Error &error);
    };

    virtual ~Surface() {}
    virtual void *nativeHandle() noexcept = 0;
    virtual Size drawableSize() const noexcept = 0;
    virtual bool start(const Callbacks &callbacks, Error &error) = 0;
    virtual void stop() noexcept = 0;
};

class Engine
{
public:
    using ErrorHandler = engine::ErrorHandler;

    ~Engine();

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    bool initialize(Surface &surface, ErrorHandler errorHandler, Error &error);
    void shutdown() noexcept;

private:
    friend class detail::EngineFactory;

    struct ErrorRelay;

    explicit Engine(std::unique_ptr<detail::Renderer> renderer);
    static bool draw(void *context, void *frame, Error &error);
    static bool resize(void *context, Size size, Error &error);
    static void surfaceFailure(void *context, const Error &error);

    bool reportFailure(const Error &error);

    std::unique_ptr<detail::Renderer> renderer_;
    Surface *surface_;
    std::shared_ptr<ErrorRelay> errorRelay_;
    bool initialized_;
    bool rendererStarted_;
    bool surfaceStarted_;
};

std::unique_ptr<Engine> createEngine();

} // namespace engine

#endif
