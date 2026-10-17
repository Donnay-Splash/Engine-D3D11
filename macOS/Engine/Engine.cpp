#include "Engine.h"
#include "EngineBackend.h"

#include <mutex>
#include <utility>

namespace engine {

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

Engine::Engine(std::unique_ptr<detail::Renderer> renderer)
    : renderer_(std::move(renderer)),
      surface_(nullptr),
      errorRelay_(),
      initialized_(false),
      rendererStarted_(false),
      surfaceStarted_(false)
{
}

Engine::~Engine()
{
    shutdown();
}

bool Engine::initialize(Surface &surface, ErrorHandler errorHandler, Error &error)
{
    if (initialized_ || renderer_ == nullptr) {
        error.message = renderer_ == nullptr
            ? "The engine has no renderer implementation."
            : "The engine is already initialized.";
        return false;
    }

    surface_ = &surface;
    errorRelay_ = std::shared_ptr<ErrorRelay>(new ErrorRelay(errorHandler));
    const std::weak_ptr<ErrorRelay> weakErrorRelay(errorRelay_);
    const ErrorHandler rendererErrorHandler = [weakErrorRelay](const Error &runtimeError) {
        const std::shared_ptr<ErrorRelay> relay = weakErrorRelay.lock();
        if (relay != nullptr) {
            relay->notify(runtimeError);
        }
    };
    rendererStarted_ = true;
    if (!renderer_->initialize(
            surface.nativeHandle(), surface.drawableSize(), rendererErrorHandler, error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to initialize.";
        }
        errorRelay_->deactivate();
        errorRelay_.reset();
        renderer_->shutdown();
        rendererStarted_ = false;
        surface_ = nullptr;
        return false;
    }

    initialized_ = true;
    surfaceStarted_ = true;
    const Surface::Callbacks callbacks = {
        this,
        &Engine::draw,
        &Engine::resize,
        &Engine::surfaceFailure,
    };
    if (!surface.start(callbacks, error)) {
        if (error.message.empty()) {
            error.message = "The render surface failed to start.";
        }
        errorRelay_->deactivate();
        initialized_ = false;
        surface.stop();
        surfaceStarted_ = false;
        renderer_->shutdown();
        rendererStarted_ = false;
        surface_ = nullptr;
        errorRelay_.reset();
        return false;
    }
    return true;
}

void Engine::shutdown() noexcept
{
    if (errorRelay_ != nullptr) {
        errorRelay_->deactivate();
    }
    if (surfaceStarted_ && surface_ != nullptr) {
        surface_->stop();
        surfaceStarted_ = false;
    }
    initialized_ = false;
    if (rendererStarted_ && renderer_ != nullptr) {
        renderer_->shutdown();
        rendererStarted_ = false;
    }
    surface_ = nullptr;
    errorRelay_.reset();
}

bool Engine::draw(void *context, void *frame, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->initialized_
        || engine->errorRelay_ == nullptr || engine->errorRelay_->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    if (!engine->renderer_->render(frame, error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to draw a frame.";
        }
        engine->reportFailure(error);
        return false;
    }
    return true;
}

bool Engine::resize(void *context, Size size, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->initialized_
        || engine->errorRelay_ == nullptr || engine->errorRelay_->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    if (!engine->renderer_->resize(size, error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to resize.";
        }
        engine->reportFailure(error);
        return false;
    }
    return true;
}

void Engine::surfaceFailure(void *context, const Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine != nullptr) {
        engine->reportFailure(error);
    }
}

bool Engine::reportFailure(const Error &error)
{
    return errorRelay_ != nullptr && errorRelay_->notify(error);
}

std::unique_ptr<Engine> detail::EngineFactory::create(
    std::unique_ptr<detail::Renderer> renderer)
{
    return std::unique_ptr<Engine>(new Engine(std::move(renderer)));
}

} // namespace engine
