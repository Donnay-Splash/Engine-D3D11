#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalFence final : public Fence
{
public:
    bool IsSignaled() const override;

    bool Wait(Error &) override;

    bool Reset(Error &error) override;

    bool Reserve(Error &error);

    void Complete();

    void Cancel();

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool signaled_ = false;
    bool pending_ = false;
};

} // namespace Metal
} // namespace Engine
