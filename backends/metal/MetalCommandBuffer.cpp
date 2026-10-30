#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalCommandBuffer::MetalCommandBuffer(NS::SharedPtr<MTL::CommandQueue> queue,
                                       MTL::CommandBuffer *commandBuffer)
    : queue_(std::move(queue)), commandBuffer_(NS::RetainPtr(commandBuffer)),
      state_(std::make_shared<SubmissionState>())
{
    state_->commandBuffer = commandBuffer_;
}

MTL::CommandBuffer *MetalCommandBuffer::Native() const noexcept
{
    return commandBuffer_.get();
}

MTL::CommandQueue *MetalCommandBuffer::NativeQueue() const noexcept
{
    return queue_.get();
}

std::shared_ptr<SubmissionState> MetalCommandBuffer::State() const
{
    return state_;
}

bool MetalCommandBuffer::Finished() const noexcept
{
    return finished_;
}

bool MetalCommandBuffer::Submitted() const noexcept
{
    return submitted_;
}

bool MetalCommandBuffer::InFlight() const
{
    std::lock_guard<std::mutex> lock(state_->mutex);
    return submitted_ && !state_->completed;
}

void MetalCommandBuffer::AddResource(const std::shared_ptr<Resource> &resource)
{
    if (resource != nullptr &&
        std::find(resources_.begin(), resources_.end(), resource) == resources_.end()) {
        resources_.push_back(resource);
    }
}

void MetalCommandBuffer::AddNativeResource(MTL::Buffer *buffer)
{
    if (buffer != nullptr) {
        nativeResources_.push_back(NS::RetainPtr(buffer));
    }
}

void MetalCommandBuffer::AddDrawable(CA::MetalDrawable *drawable)
{
    if (drawable == nullptr) {
        return;
    }

    const auto found = std::find_if(drawables_.begin(), drawables_.end(),
                                    [drawable](const NS::SharedPtr<CA::MetalDrawable> &item) {
                                        return item.get() == drawable;
                                    });
    if (found == drawables_.end()) {
        drawables_.push_back(NS::RetainPtr(drawable));
    }
}

void MetalCommandBuffer::MarkPresented(CA::MetalDrawable *drawable)
{
    if (drawable != nullptr &&
        std::none_of(presentedDrawables_.begin(), presentedDrawables_.end(),
                     [drawable](const NS::SharedPtr<CA::MetalDrawable> &item) {
                         return item.get() == drawable;
                     })) {
        presentedDrawables_.push_back(NS::RetainPtr(drawable));
    }
}

bool MetalCommandBuffer::HasPresented(CA::MetalDrawable *drawable) const
{
    return std::any_of(presentedDrawables_.begin(), presentedDrawables_.end(),
                       [drawable](const NS::SharedPtr<CA::MetalDrawable> &item) {
                           return item.get() == drawable;
                       });
}

const std::vector<NS::SharedPtr<CA::MetalDrawable>> &MetalCommandBuffer::Drawables() const
{
    return drawables_;
}

void MetalCommandBuffer::MarkFinished()
{
    finished_ = true;
}

void MetalCommandBuffer::MarkSubmitted()
{
    submitted_ = true;
}

void MetalCommandBuffer::Complete(NS::Error *nativeError)
{
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        if (nativeError != nullptr) {
            const char *message = nativeError->localizedDescription()->utf8String();
            if (message != nullptr) {
                state_->error = message;
            }
        }
        state_->completed = true;
    }
    resources_.clear();
    nativeResources_.clear();
    drawables_.clear();
}

bool MetalCommandBuffer::Reset(Error &error)
{
    AutoreleasePool pool;
    if (InFlight()) {
        SetError(error, "A Metal command buffer cannot be reset before GPU completion.");
        return false;
    }
    if (submitted_ && !state_->completed) {
        SetError(error, "A Metal command buffer is still in flight.");
        return false;
    }
    if (!finished_) {
        SetError(error, "A Metal command buffer must be finished before reset.");
        return false;
    }
    MTL::CommandBuffer *next = queue_->commandBuffer();
    if (next == nullptr) {
        SetError(error, "Metal failed to create a replacement command buffer.");
        return false;
    }
    commandBuffer_ = NS::RetainPtr(next);
    state_ = std::make_shared<SubmissionState>();
    state_->commandBuffer = commandBuffer_;
    resources_.clear();
    nativeResources_.clear();
    drawables_.clear();
    presentedDrawables_.clear();
    finished_ = false;
    submitted_ = false;
    return true;
}

} // namespace Metal
} // namespace Engine
