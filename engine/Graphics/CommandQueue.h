#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

#include <memory>
#include <vector>

namespace Engine {
struct Error;
namespace Graphics {

class CommandBuffer;
class CommandContext;
class Fence;
class Submission;

class CommandQueue : public Resource
{
public:
    virtual QueueType Type() const noexcept = 0;
    // Graphics queues must support copy encoders, and dependencies may cross queue submissions.
    virtual std::shared_ptr<CommandContext> CreateContext(::Engine::Error &error) = 0;
    virtual std::shared_ptr<Submission> Submit(
        const std::shared_ptr<CommandBuffer> &commandBuffer,
        const std::vector<std::shared_ptr<Submission>> &waitFor,
        const std::vector<std::shared_ptr<Fence>> &signal,
        ::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
