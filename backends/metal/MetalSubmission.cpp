#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalSubmission::MetalSubmission(std::shared_ptr<SubmissionState> state,
                                 std::shared_ptr<MetalCommandBuffer> commandBuffer)
    : state_(std::move(state)), commandBuffer_(std::move(commandBuffer))
{
}

bool MetalSubmission::IsComplete() const
{
    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->completed;
}

const std::shared_ptr<MetalCommandBuffer> &MetalSubmission::Commands() const noexcept
{
    return commandBuffer_;
}

const std::shared_ptr<SubmissionState> &MetalSubmission::State() const noexcept
{
    return state_;
}

bool MetalSubmission::Wait(Error &error)
{
    AutoreleasePool pool;
    state_->commandBuffer->waitUntilCompleted();
    std::lock_guard<std::mutex> lock(state_->mutex);
    if (!state_->error.empty()) {
        error.message = state_->error;
        return false;
    }
    if (state_->commandBuffer->status() == MTL::CommandBufferStatusError) {
        SetError(error, "Metal command submission failed.");
        return false;
    }
    return true;
}

} // namespace Metal
} // namespace Engine
