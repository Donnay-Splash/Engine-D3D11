#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalCommandBuffer final : public CommandBuffer
{
public:
    MetalCommandBuffer(NS::SharedPtr<MTL::CommandQueue> queue, MTL::CommandBuffer *commandBuffer);

    MTL::CommandBuffer *Native() const noexcept;

    MTL::CommandQueue *NativeQueue() const noexcept;

    std::shared_ptr<SubmissionState> State() const;

    bool Finished() const noexcept;

    bool Submitted() const noexcept;

    bool InFlight() const;

    void AddResource(const std::shared_ptr<Resource> &resource);

    void AddNativeResource(MTL::Buffer *buffer);

    void AddDrawable(CA::MetalDrawable *drawable);

    void MarkPresented(CA::MetalDrawable *drawable);

    bool HasPresented(CA::MetalDrawable *drawable) const;

    const std::vector<NS::SharedPtr<CA::MetalDrawable>> &Drawables() const;

    void MarkFinished();

    void MarkSubmitted();

    void Complete(NS::Error *nativeError);

    bool Reset(Error &error) override;

private:
    NS::SharedPtr<MTL::CommandQueue> queue_;
    NS::SharedPtr<MTL::CommandBuffer> commandBuffer_;
    std::shared_ptr<SubmissionState> state_;
    std::vector<std::shared_ptr<Resource>> resources_;
    std::vector<NS::SharedPtr<MTL::Buffer>> nativeResources_;
    std::vector<NS::SharedPtr<CA::MetalDrawable>> drawables_;
    std::vector<NS::SharedPtr<CA::MetalDrawable>> presentedDrawables_;
    bool finished_ = false;
    bool submitted_ = false;
    friend class MetalCommandContext;
    friend class MetalQueue;
};

} // namespace Metal
} // namespace Engine
