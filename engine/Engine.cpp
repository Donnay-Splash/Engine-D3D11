#include "Engine.h"
#include "Device.h"

#include <mutex>
#include <utility>

namespace Engine {

struct Engine::ErrorRelay
{
    explicit ErrorRelay(const ErrorHandler &errorHandler)
        : handler(errorHandler), failed(false)
    {
    }

    bool notify(const Error &error)
    {
        ErrorHandler callback;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (failed) {
                return false;
            }
            failed = true;
            callback = handler;
        }
        if (callback) {
            callback(error);
        }
        return true;
    }

    bool hasFailed()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return failed;
    }

    void deactivate()
    {
        std::lock_guard<std::mutex> lock(mutex);
        failed = true;
        handler = ErrorHandler();
    }

    std::mutex mutex;
    ErrorHandler handler;
    bool failed;
};

Engine::Engine(std::unique_ptr<Device> device)
    : m_Device(std::move(device)),
      m_Surface(nullptr),
      m_ErrorRelay(),
      m_Initialized(false),
      m_RendererStarted(false),
      m_SurfaceStarted(false)
{
}

Engine::~Engine()
{
    Shutdown();
}

bool Engine::Initialize(Surface &surface, ErrorHandler errorHandler, Error &error)
{
    if (m_Initialized || m_Device == nullptr) {
        error.message = m_Device == nullptr
            ? "The engine has no renderer implementation."
            : "The engine is already initialized.";
        return false;
    }

    m_Surface = &surface;
    m_ErrorRelay = std::shared_ptr<ErrorRelay>(new ErrorRelay(errorHandler));
    const std::weak_ptr<ErrorRelay> weakErrorRelay(m_ErrorRelay);
    const ErrorHandler rendererErrorHandler = [weakErrorRelay](const Error &runtimeError) {
        const std::shared_ptr<ErrorRelay> relay = weakErrorRelay.lock();
        if (relay != nullptr) {
            relay->notify(runtimeError);
        }
    };
    m_RendererStarted = true;
    if (!m_Device->Initialize(
            surface.NativeHandle(),
            surface.DrawableSize(),
            rendererErrorHandler,
            error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to initialize.";
        }
        m_ErrorRelay->deactivate();
        m_ErrorRelay.reset();
        m_Device->Shutdown();
        m_RendererStarted = false;
        m_Surface = nullptr;
        return false;
    }

    m_Initialized = true;
    m_SurfaceStarted = true;
    const Surface::Callbacks callbacks = {
        this,
        &Engine::Draw,
        &Engine::Resize,
        &Engine::SurfaceFailure,
    };
    if (!surface.Start(callbacks, error)) {
        if (error.message.empty()) {
            error.message = "The render surface failed to start.";
        }
        m_ErrorRelay->deactivate();
        m_Initialized = false;
        surface.Stop();
        m_SurfaceStarted = false;
        m_Device->Shutdown();
        m_RendererStarted = false;
        m_Surface = nullptr;
        m_ErrorRelay.reset();
        return false;
    }
    return true;
}

void Engine::Shutdown() noexcept
{
    if (m_ErrorRelay != nullptr) {
        m_ErrorRelay->deactivate();
    }
    if (m_SurfaceStarted && m_Surface != nullptr) {
        m_Surface->Stop();
        m_SurfaceStarted = false;
    }
    m_Initialized = false;
    if (m_RendererStarted && m_Device != nullptr) {
        m_Device->Shutdown();
        m_RendererStarted = false;
    }
    m_Surface = nullptr;
    m_ErrorRelay.reset();
}

bool Engine::Draw(void *context, void *frame, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->m_Initialized
        || engine->m_ErrorRelay == nullptr || engine->m_ErrorRelay->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    (void)frame;
    return true;
}

bool Engine::Resize(void *context, Size size, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->m_Initialized
        || engine->m_ErrorRelay == nullptr || engine->m_ErrorRelay->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    if (!engine->m_Device->Resize(size, error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to resize.";
        }
        engine->ReportFailure(error);
        return false;
    }
    return true;
}

void Engine::SurfaceFailure(void *context, const Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine != nullptr) {
        engine->ReportFailure(error);
    }
}

bool Engine::ReportFailure(const Error &error)
{
    return m_ErrorRelay != nullptr && m_ErrorRelay->notify(error);
}

std::unique_ptr<Engine> EngineFactory::create(
    std::unique_ptr<Device> device)
{
    return std::unique_ptr<Engine>(new Engine(std::move(device)));
}

} // namespace engine
