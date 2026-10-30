#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalCommandContext final : public CommandContext,
                                  public std::enable_shared_from_this<MetalCommandContext>
{
public:
    explicit MetalCommandContext(std::shared_ptr<MetalQueue> queue);

    bool Valid() const noexcept;

    std::unique_ptr<RenderPassEncoder> BeginRenderPass(const RenderPassDesc &description,
                                                       Error &error) override;
    std::unique_ptr<ComputeEncoder> BeginCompute(Error &error) override;
    std::unique_ptr<CopyEncoder> BeginCopy(Error &error) override;
    bool Barrier(const std::shared_ptr<Resource> &resource, const ResourceBarrierDesc &description,
                 Error &error) override;
    std::shared_ptr<CommandBuffer> Finish(Error &error) override;

    bool BeginEncoder(Error &error);

    void EndEncoder();

    void Keep(const std::shared_ptr<Resource> &resource);

    void DeferError(const char *message);

    MTL::Device *Device() const noexcept;

    MTL::CommandBuffer *Native() const noexcept;

    QueueType Type() const noexcept;

private:
    std::shared_ptr<MetalQueue> queue_;
    std::shared_ptr<MetalCommandBuffer> commandBuffer_;
    std::string deferredError_;
    bool activeEncoder_ = false;
    bool finished_ = false;
};

} // namespace Metal
} // namespace Engine
