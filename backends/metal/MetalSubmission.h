#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalSubmission final : public Submission
{
public:
    MetalSubmission(std::shared_ptr<SubmissionState> state,
                    std::shared_ptr<MetalCommandBuffer> commandBuffer);

    bool IsComplete() const override;

    bool Wait(Error &error) override;
    const std::shared_ptr<MetalCommandBuffer> &Commands() const noexcept;

    const std::shared_ptr<SubmissionState> &State() const noexcept;

private:
    std::shared_ptr<SubmissionState> state_;
    std::shared_ptr<MetalCommandBuffer> commandBuffer_;
};

} // namespace Metal
} // namespace Engine
