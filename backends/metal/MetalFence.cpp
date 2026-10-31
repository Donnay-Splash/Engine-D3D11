#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

bool MetalFence::IsSignaled() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return signaled_;
}

bool MetalFence::Wait(Error &)
{
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return signaled_; });
    return true;
}

bool MetalFence::Reset(Error &error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pending_) {
        SetError(error, "A pending Metal fence cannot be reset.");
        return false;
    }
    signaled_ = false;
    return true;
}

bool MetalFence::Reserve(Error &error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pending_) {
        SetError(error, "A Metal fence cannot be signaled by overlapping submissions.");
        return false;
    }
    pending_ = true;
    signaled_ = false;
    return true;
}

void MetalFence::Complete()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_ = false;
        signaled_ = true;
    }
    condition_.notify_all();
}

void MetalFence::Cancel()
{
    std::lock_guard<std::mutex> lock(mutex_);
    pending_ = false;
}

} // namespace Metal
} // namespace Engine
