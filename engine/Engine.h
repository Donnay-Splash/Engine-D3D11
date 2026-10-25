#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace Engine {

class Engine;
class Device;
class EngineFactory;

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
        bool (*Draw)(void *context, void *frame, Error &error);
        bool (*Resize)(void *context, Size size, Error &error);
        void (*Failure)(void *context, const Error &error);
    };

    virtual ~Surface() {}
    virtual void *NativeHandle() noexcept = 0;
    virtual Size DrawableSize() const noexcept = 0;
    virtual bool Start(const Callbacks &callbacks, Error &error) = 0;
    virtual void Stop() noexcept = 0;
};

class Engine
{
public:
    using ErrorHandler = ErrorHandler;

    ~Engine();

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    bool Initialize(Surface &surface, ErrorHandler errorHandler, Error &error);
    void Shutdown() noexcept;

private:
    friend class EngineFactory;

    struct ErrorRelay;

    explicit Engine(std::unique_ptr<Device> renderer);
    static bool Draw(void *context, void *frame, Error &error);
    static bool Resize(void *context, Size size, Error &error);
    static void SurfaceFailure(void *context, const Error &error);

    bool ReportFailure(const Error &error);

    std::unique_ptr<Device> m_Device;
    Surface *m_Surface;
    std::shared_ptr<ErrorRelay> m_ErrorRelay;
    bool m_Initialized;
    bool m_RendererStarted;
    bool m_SurfaceStarted;
};

} // namespace engine
