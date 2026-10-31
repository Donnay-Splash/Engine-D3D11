#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalRenderEncoder final : public RenderPassEncoder
{
public:
    MetalRenderEncoder(std::shared_ptr<MetalCommandContext> context,
                       MTL::RenderCommandEncoder *encoder, std::vector<TextureFormat> colorFormats,
                       TextureFormat depthFormat, std::uint32_t sampleCount);

    ~MetalRenderEncoder() override;

    bool End(Error &error) override;

    bool SetPipeline(const std::shared_ptr<GraphicsPipeline> &pipeline, Error &error) override;

    bool SetBindingSet(std::uint32_t index, const std::shared_ptr<BindingSet> &bindings,
                       Error &error) override;

    bool SetViewport(const Viewport &viewport, Error &error) override;

    bool SetVertexBuffer(std::uint32_t slot, const std::shared_ptr<Buffer> &buffer,
                         std::size_t offset, Error &error) override;

    bool SetIndexBuffer(const std::shared_ptr<Buffer> &buffer, std::size_t offset,
                        IndexFormat format, Error &error) override;

    void Draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex,
              std::uint32_t firstInstance) override;

    void DrawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount,
                     std::uint32_t firstIndex, std::int32_t baseVertex,
                     std::uint32_t firstInstance) override;

private:
    std::shared_ptr<MetalCommandContext> context_;
    NS::SharedPtr<MTL::RenderCommandEncoder> encoder_;
    std::vector<TextureFormat> colorFormats_;
    TextureFormat depthFormat_;
    std::uint32_t sampleCount_;
    std::shared_ptr<MetalGraphicsPipeline> pipeline_;
    std::shared_ptr<MetalBuffer> indexBuffer_;
    std::size_t indexOffset_ = 0;
    IndexFormat indexFormat_ = IndexFormat::UInt16;
    bool ended_ = false;
};

} // namespace Metal
} // namespace Engine
