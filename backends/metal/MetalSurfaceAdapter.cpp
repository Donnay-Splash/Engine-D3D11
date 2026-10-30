#include "MetalSurfaceAdapter.h"

#include <Foundation/NSAutoreleasePool.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <condition_variable>
#include <exception>
#include <mutex>
#include <string>

namespace Engine {
namespace Metal {
namespace {

class AutoreleasePool final
{
public:
    AutoreleasePool() : pool_(NS::TransferPtr(NS::AutoreleasePool::alloc()->init())) {}

private:
    NS::SharedPtr<NS::AutoreleasePool> pool_;
};

thread_local const class MetalSurfaceImpl *dispatchingSurface = nullptr;

class MetalSurfaceImpl final : public MetalSurfaceAdapter
{
public:
    explicit MetalSurfaceImpl(CA::MetalLayer *layer)
        : layer_(NS::RetainPtr(layer))
    {
    }

    ~MetalSurfaceImpl() override
    {
        Stop();
    }

    Size DrawableSize() const noexcept override
    {
        const CGSize size = layer_->drawableSize();
        return {
            size.width > 0.0 ? static_cast<std::uint32_t>(size.width) : 0,
            size.height > 0.0 ? static_cast<std::uint32_t>(size.height) : 0,
        };
    }

    CA::MetalLayer *Layer() const noexcept override
    {
        return layer_.get();
    }

    bool Start(const Callbacks &callbacks, Error &error) override
    {
        if (callbacks.Draw == nullptr || callbacks.Resize == nullptr) {
            error.message = "A Metal surface requires draw and resize callbacks.";
            return false;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if (started_) {
            error.message = "The Metal surface is already started.";
            return false;
        }
        callbacks_ = callbacks;
        started_ = true;
        return true;
    }

    void Stop() noexcept override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        started_ = false;
        callbacks_ = {};
        if (dispatchingSurface != this) {
            condition_.wait(lock, [this] { return activeDispatches_ == 0; });
        }
    }

    bool DispatchDraw(Error &error)
    {
        Callbacks callbacks;
        if (!BeginDispatch(callbacks, error)) {
            return false;
        }
        const MetalSurfaceImpl *previous = dispatchingSurface;
        dispatchingSurface = this;
        bool succeeded = false;
        try {
            succeeded = callbacks.Draw(callbacks.context, error);
        } catch (const std::exception &exception) {
            error.message = std::string("The Metal draw callback threw an exception: ")
                + exception.what();
        } catch (...) {
            error.message = "The Metal draw callback threw an unknown exception.";
        }
        dispatchingSurface = previous;
        FinishDispatch();
        if (!succeeded) {
            ReportFailure(callbacks, error);
        }
        return succeeded;
    }

    bool DispatchResize(Size size, Error &error)
    {
        Callbacks callbacks;
        if (!BeginDispatch(callbacks, error)) {
            return false;
        }
        const MetalSurfaceImpl *previous = dispatchingSurface;
        dispatchingSurface = this;
        bool succeeded = false;
        try {
            succeeded = callbacks.Resize(callbacks.context, size, error);
        } catch (const std::exception &exception) {
            error.message = std::string("The Metal resize callback threw an exception: ")
                + exception.what();
        } catch (...) {
            error.message = "The Metal resize callback threw an unknown exception.";
        }
        dispatchingSurface = previous;
        FinishDispatch();
        if (!succeeded) {
            ReportFailure(callbacks, error);
        }
        return succeeded;
    }

private:
    bool BeginDispatch(Callbacks &callbacks, Error &error)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!started_) {
            error.message = "The Metal surface is not started.";
            return false;
        }
        callbacks = callbacks_;
        ++activeDispatches_;
        return true;
    }

    void FinishDispatch()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            --activeDispatches_;
        }
        condition_.notify_all();
    }

    static void ReportFailure(const Callbacks &callbacks, const Error &error)
    {
        if (callbacks.Failure == nullptr) {
            return;
        }
        try {
            callbacks.Failure(callbacks.context, error);
        } catch (...) {
        }
    }

    NS::SharedPtr<CA::MetalLayer> layer_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    Callbacks callbacks_{};
    std::size_t activeDispatches_ = 0;
    bool started_ = false;
};

} // namespace

std::unique_ptr<Surface> CreateMetalSurface(CA::MetalLayer *layer)
{
    if (layer == nullptr) {
        return nullptr;
    }
    return std::unique_ptr<Surface>(new MetalSurfaceImpl(layer));
}

bool DispatchMetalDraw(Surface &surface, Error &error)
{
    auto *metalSurface = dynamic_cast<MetalSurfaceImpl *>(&surface);
    if (metalSurface == nullptr) {
        error.message = "The surface is not owned by the Metal backend.";
        return false;
    }
    AutoreleasePool pool;
    return metalSurface->DispatchDraw(error);
}

bool DispatchMetalResize(Surface &surface, Size size, Error &error)
{
    auto *metalSurface = dynamic_cast<MetalSurfaceImpl *>(&surface);
    if (metalSurface == nullptr) {
        error.message = "The surface is not owned by the Metal backend.";
        return false;
    }
    AutoreleasePool pool;
    return metalSurface->DispatchResize(size, error);
}

} // namespace Metal
} // namespace Engine
