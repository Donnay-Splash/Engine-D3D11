#pragma once

#include "CommandBuffer.h"
#include "GraphicsTypes.h"
#include "Resource.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class ComputeEncoder;
class CopyEncoder;
class RenderPassEncoder;
class Submission;

class CommandContext : public Resource
{
public:
    virtual std::unique_ptr<RenderPassEncoder> BeginRenderPass(
        const RenderPassDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::unique_ptr<ComputeEncoder> BeginCompute(
        ::Engine::Error &error) = 0;
    virtual std::unique_ptr<CopyEncoder> BeginCopy(
        ::Engine::Error &error) = 0;
    virtual bool Barrier(
        const std::shared_ptr<Resource> &resource,
        const ResourceBarrierDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<CommandBuffer> Finish(::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
