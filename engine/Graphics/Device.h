#pragma once

#include "DeviceCapabilities.h"
#include "GraphicsTypes.h"
#include "Resource.h"

#include <cstddef>
#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Buffer;
class BufferView;
class BindingLayout;
class BindingSet;
class CommandQueue;
class ComputePipeline;
class GraphicsPipeline;
class Fence;
class MemoryAllocator;
class PresentationSurface;
class Sampler;
class Shader;
class Swapchain;
class Texture;
class TextureView;
struct BindingLayoutDesc;
struct SwapchainDesc;

class Device : public Resource
{
public:
    virtual const DeviceCapabilities &Capabilities() const noexcept = 0;
    virtual std::shared_ptr<Buffer> CreateBuffer(
        const BufferDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<Texture> CreateTexture(
        const TextureDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<Shader> CreateShader(
        const ShaderDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<BufferView> CreateBufferView(
        const std::shared_ptr<Buffer> &buffer,
        std::size_t offset,
        std::size_t size,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<TextureView> CreateTextureView(
        const std::shared_ptr<Texture> &texture,
        const ResourceRange &range,
        TextureFormat format,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<Sampler> CreateSampler(
        const SamplerDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<BindingLayout> CreateBindingLayout(
        const BindingLayoutDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<BindingSet> CreateBindingSet(
        const std::shared_ptr<BindingLayout> &layout,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<GraphicsPipeline> CreateGraphicsPipeline(
        const struct GraphicsPipelineDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<Swapchain> CreateSwapchain(
        PresentationSurface &surface,
        const SwapchainDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<ComputePipeline> CreateComputePipeline(
        const struct ComputePipelineDesc &description,
        ::Engine::Error &error) = 0;
    virtual std::shared_ptr<CommandQueue> GetQueue(QueueType type) = 0;
    virtual std::shared_ptr<Fence> CreateFence(::Engine::Error &error) = 0;
    virtual std::shared_ptr<MemoryAllocator> GetMemoryAllocator() = 0;
};

} // namespace Graphics
} // namespace Engine
