#pragma once

#include "BindingSet.h"
#include "CommandEncoder.h"
#include "GraphicsPipeline.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Buffer;

class RenderPassEncoder : public CommandEncoder
{
public:
    virtual bool SetPipeline(
        const std::shared_ptr<GraphicsPipeline> &pipeline,
        ::Engine::Error &error) = 0;
    virtual bool SetBindingSet(
        std::uint32_t index,
        const std::shared_ptr<BindingSet> &bindings,
        ::Engine::Error &error) = 0;
    virtual bool SetViewport(const Viewport &viewport, ::Engine::Error &error) = 0;
    virtual bool SetVertexBuffer(
        std::uint32_t slot,
        const std::shared_ptr<Buffer> &buffer,
        std::size_t offset,
        ::Engine::Error &error) = 0;
    virtual bool SetIndexBuffer(
        const std::shared_ptr<Buffer> &buffer,
        std::size_t offset,
        IndexFormat format,
        ::Engine::Error &error) = 0;
    virtual void Draw(
        std::uint32_t vertexCount,
        std::uint32_t instanceCount = 1,
        std::uint32_t firstVertex = 0,
        std::uint32_t firstInstance = 0) = 0;
    virtual void DrawIndexed(
        std::uint32_t indexCount,
        std::uint32_t instanceCount = 1,
        std::uint32_t firstIndex = 0,
        std::int32_t baseVertex = 0,
        std::uint32_t firstInstance = 0) = 0;
};

} // namespace Graphics
} // namespace Engine
