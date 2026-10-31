#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalQueue::MetalQueue(QueueType type, NS::SharedPtr<MTL::CommandQueue> queue, MTL::Device *device)
    : type_(type), queue_(std::move(queue)), device_(device)
{
}

QueueType MetalQueue::Type() const noexcept
{
    return type_;
}

std::shared_ptr<Submission>
MetalQueue::Submit(const std::shared_ptr<CommandBuffer> &commandBuffer,
                   const std::vector<std::shared_ptr<Submission>> &waitFor,
                   const std::vector<std::shared_ptr<Fence>> &signal, Error &error)
{
    AutoreleasePool pool;
    auto metalCommandBuffer = std::dynamic_pointer_cast<MetalCommandBuffer>(commandBuffer);
    if (metalCommandBuffer == nullptr || !metalCommandBuffer->Finished() ||
        metalCommandBuffer->Submitted()) {
        SetError(error, "Only a finished, unsubmitted Metal command buffer can be submitted.");
        return nullptr;
    }
    for (const std::shared_ptr<Submission> &dependency : waitFor) {
        if (dependency == nullptr || !dependency->Wait(error)) {
            if (error.message.empty()) {
                SetError(error, "A Metal submission dependency failed.");
            }
            return nullptr;
        }
    }

    std::vector<std::shared_ptr<MetalFence>> fences;
    for (const std::shared_ptr<Fence> &baseFence : signal) {
        auto fence = std::dynamic_pointer_cast<MetalFence>(baseFence);
        if (fence == nullptr || !fence->Reserve(error)) {
            for (const std::shared_ptr<MetalFence> &reserved : fences) {
                reserved->Cancel();
            }
            if (error.message.empty()) {
                SetError(error, "A signal fence is not owned by the Metal backend.");
            }
            return nullptr;
        }
        fences.push_back(std::move(fence));
    }

    for (const NS::SharedPtr<CA::MetalDrawable> &drawable : metalCommandBuffer->Drawables()) {
        metalCommandBuffer->Native()->presentDrawable(drawable.get());
        metalCommandBuffer->MarkPresented(drawable.get());
    }
    std::shared_ptr<SubmissionState> state = metalCommandBuffer->State();
    metalCommandBuffer->MarkSubmitted();
    metalCommandBuffer->Native()->addCompletedHandler(
        [metalCommandBuffer, state, fences](MTL::CommandBuffer *completed) {
            metalCommandBuffer->Complete(completed->error());
            for (const std::shared_ptr<MetalFence> &fence : fences) {
                fence->Complete();
            }
        });
    metalCommandBuffer->Native()->commit();
    return std::make_shared<MetalSubmission>(std::move(state), std::move(metalCommandBuffer));
}

MTL::Device *MetalQueue::Device() const noexcept
{
    return device_;
}

const NS::SharedPtr<MTL::CommandQueue> &MetalQueue::Native() const noexcept
{
    return queue_;
}

std::shared_ptr<CommandContext> MetalQueue::CreateContext(Error &error)
{
    AutoreleasePool pool;
    std::shared_ptr<MetalCommandContext> context =
        std::make_shared<MetalCommandContext>(shared_from_this());
    if (!context->Valid()) {
        SetError(error, "Metal failed to allocate a command buffer.");
        return nullptr;
    }
    return context;
}

} // namespace Metal
} // namespace Engine
