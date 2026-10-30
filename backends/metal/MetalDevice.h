#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalDevice final : public Device
{
public:
    explicit MetalDevice(NS::SharedPtr<MTL::Device> device);

    const DeviceCapabilities &Capabilities() const noexcept override;

    std::shared_ptr<Buffer> CreateBuffer(const BufferDesc &description, Error &error) override;

    std::shared_ptr<Texture> CreateTexture(const TextureDesc &description, Error &error) override;

    std::shared_ptr<Shader> CreateShader(const ShaderDesc &description, Error &error) override;

    std::shared_ptr<BufferView> CreateBufferView(const std::shared_ptr<Buffer> &buffer,
                                                 std::size_t offset, std::size_t size,
                                                 Error &error) override;

    std::shared_ptr<TextureView> CreateTextureView(const std::shared_ptr<Texture> &texture,
                                                   const ResourceRange &range, TextureFormat format,
                                                   Error &error) override;

    std::shared_ptr<Sampler> CreateSampler(const SamplerDesc &description, Error &error) override;

    std::shared_ptr<BindingLayout> CreateBindingLayout(const BindingLayoutDesc &description,
                                                       Error &error) override;

    std::shared_ptr<BindingSet> CreateBindingSet(const std::shared_ptr<BindingLayout> &layout,
                                                 Error &error) override;

    std::shared_ptr<GraphicsPipeline>
    CreateGraphicsPipeline(const GraphicsPipelineDesc &description, Error &error) override;

    std::shared_ptr<Swapchain> CreateSwapchain(PresentationSurface &surface,
                                               const SwapchainDesc &description,
                                               Error &error) override;

    std::shared_ptr<ComputePipeline> CreateComputePipeline(const ComputePipelineDesc &description,
                                                           Error &error) override;

    std::shared_ptr<CommandQueue> GetQueue(QueueType type) override;

    std::shared_ptr<Fence> CreateFence(Error &) override;

    std::shared_ptr<MemoryAllocator> GetMemoryAllocator() override;

    MTL::Device *Native() const noexcept;

private:
    NS::SharedPtr<MTL::Device> device_;
    DeviceCapabilities capabilities_;
    NS::SharedPtr<MTL::CommandQueue> nativeQueue_;
    std::array<std::shared_ptr<MetalQueue>, 3> queues_;
    std::mutex queueMutex_;
    std::shared_ptr<MetalMemoryAllocator> allocator_;
};

} // namespace Metal
} // namespace Engine
