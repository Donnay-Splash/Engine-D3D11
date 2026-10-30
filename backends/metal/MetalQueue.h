#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalQueue final : public CommandQueue, public std::enable_shared_from_this<MetalQueue>
{
public:
    MetalQueue(QueueType type, NS::SharedPtr<MTL::CommandQueue> queue, MTL::Device *device);

    QueueType Type() const noexcept override;

    std::shared_ptr<CommandContext> CreateContext(Error &error) override;

    std::shared_ptr<Submission> Submit(const std::shared_ptr<CommandBuffer> &commandBuffer,
                                       const std::vector<std::shared_ptr<Submission>> &waitFor,
                                       const std::vector<std::shared_ptr<Fence>> &signal,
                                       Error &error) override;

    MTL::Device *Device() const noexcept;

    const NS::SharedPtr<MTL::CommandQueue> &Native() const noexcept;

private:
    QueueType type_;
    NS::SharedPtr<MTL::CommandQueue> queue_;
    MTL::Device *device_;
};

} // namespace Metal
} // namespace Engine
