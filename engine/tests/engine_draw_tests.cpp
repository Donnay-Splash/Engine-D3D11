#include "Core/Math.h"
#include "Engine.h"
#include "EngineFactory.h"
#include "Graphics/Adapter.h"
#include "Graphics/BindingLayout.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/BufferView.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandContext.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/ComputeEncoder.h"
#include "Graphics/ComputePipeline.h"
#include "Graphics/CopyEncoder.h"
#include "Graphics/Device.h"
#include "Graphics/Fence.h"
#include "Graphics/GraphicsInstance.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Submission.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureView.h"
#include "ShaderLibrary.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool condition, const char *message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct FakeState
{
    std::vector<std::string> events;
    std::vector<Engine::Graphics::ResourceUsage> barrierTargets;
    std::vector<std::shared_ptr<class FakeBuffer>> buffers;
    std::vector<std::size_t> waitCounts;
    std::weak_ptr<class FakeSubmission> uploadSubmission;
    std::weak_ptr<class FakeSwapchain> swapchain;
    std::shared_ptr<class FakeGraphicsPipeline> pipeline;
    std::vector<std::uint32_t> boundSlots;
    Engine::Graphics::Viewport viewport{};
    std::uint32_t drawVertexCount = 0;
    std::uint32_t acquireCount = 0;
    std::uint32_t presentCount = 0;
    std::uint32_t deviceCreateCount = 0;
    bool presentSucceeds = true;
};

class FakeBuffer final : public Engine::Graphics::Buffer
{
public:
    explicit FakeBuffer(const Engine::Graphics::BufferDesc &description)
        : description_(description), bytes_(description.size)
    {
    }

    const Engine::Graphics::BufferDesc &Description() const noexcept override
    {
        return description_;
    }

    bool Map(void *&data, Engine::Error &) override
    {
        data = bytes_.data();
        return !bytes_.empty();
    }

    void Unmap() noexcept override {}

    const std::vector<std::uint8_t> &Bytes() const noexcept { return bytes_; }

    std::vector<std::uint8_t> &Bytes() noexcept { return bytes_; }

private:
    Engine::Graphics::BufferDesc description_;
    std::vector<std::uint8_t> bytes_;
};

class FakeTexture final : public Engine::Graphics::Texture
{
public:
    explicit FakeTexture(const Engine::Graphics::TextureDesc &description)
        : description_(description)
    {
    }

    const Engine::Graphics::TextureDesc &Description() const noexcept override
    {
        return description_;
    }

private:
    Engine::Graphics::TextureDesc description_;
};

class FakeTextureView final : public Engine::Graphics::TextureView
{
public:
    explicit FakeTextureView(std::shared_ptr<Engine::Graphics::Texture> texture)
        : texture_(std::move(texture))
    {
    }

    std::shared_ptr<Engine::Graphics::Texture> GetTexture() const override
    {
        return texture_;
    }

    const Engine::Graphics::ResourceRange &Range() const noexcept override
    {
        return range_;
    }

    Engine::Graphics::TextureFormat Format() const noexcept override
    {
        return texture_->Description().format;
    }

private:
    std::shared_ptr<Engine::Graphics::Texture> texture_;
    Engine::Graphics::ResourceRange range_;
};

class FakeSwapchainImage final : public Engine::Graphics::SwapchainImage
{
public:
    explicit FakeSwapchainImage(std::shared_ptr<Engine::Graphics::TextureView> view)
        : view_(std::move(view))
    {
    }

    std::shared_ptr<Engine::Graphics::TextureView> View() const override
    {
        return view_;
    }

private:
    std::shared_ptr<Engine::Graphics::TextureView> view_;
};

class FakeSwapchain final : public Engine::Graphics::Swapchain
{
public:
    FakeSwapchain(
        std::shared_ptr<FakeState> state,
        const Engine::Graphics::SwapchainDesc &description)
        : state_(std::move(state)), description_(description)
    {
        Engine::Graphics::TextureDesc textureDescription;
        textureDescription.format = description.format;
        textureDescription.extent = description.extent;
        textureDescription.usage = Engine::Graphics::TextureUsage::ColorAttachment;
        auto texture = std::make_shared<FakeTexture>(textureDescription);
        image_ = std::make_shared<FakeSwapchainImage>(
            std::make_shared<FakeTextureView>(texture));
    }

    const Engine::Graphics::SwapchainDesc &Description() const noexcept override
    {
        return description_;
    }

    std::shared_ptr<Engine::Graphics::SwapchainImage> AcquireNextImage(
        Engine::Error &) override
    {
        ++state_->acquireCount;
        state_->events.push_back("acquire");
        return image_;
    }

    bool Present(
        const std::shared_ptr<Engine::Graphics::SwapchainImage> &,
        const std::shared_ptr<Engine::Graphics::Submission> &submission,
        Engine::Error &error) override
    {
        state_->events.push_back("present");
        ++state_->presentCount;
        if (!state_->presentSucceeds || submission == nullptr) {
            error.message = "fake present failed";
            return false;
        }
        return true;
    }

    bool Resize(Engine::Graphics::Extent3D extent, Engine::Error &) override
    {
        description_.extent = extent;
        state_->events.push_back("resize");
        return true;
    }

private:
    std::shared_ptr<FakeState> state_;
    Engine::Graphics::SwapchainDesc description_;
    std::shared_ptr<FakeSwapchainImage> image_;
};

class FakeShader final : public Engine::Graphics::Shader
{
public:
    explicit FakeShader(Engine::Graphics::ShaderStage stage)
        : stage_(stage)
    {
    }

    Engine::Graphics::ShaderStage Stage() const noexcept override { return stage_; }
    const std::string &EntryPoint() const noexcept override { return entryPoint_; }

private:
    Engine::Graphics::ShaderStage stage_;
    std::string entryPoint_ = "main";
};

class FakeBindingLayout final : public Engine::Graphics::BindingLayout
{
public:
    explicit FakeBindingLayout(const Engine::Graphics::BindingLayoutDesc &description)
        : bindings_(description.bindings)
    {
    }

    const std::vector<Engine::Graphics::BindingDesc> &Bindings() const noexcept override
    {
        return bindings_;
    }

private:
    std::vector<Engine::Graphics::BindingDesc> bindings_;
};

class FakeBufferView final : public Engine::Graphics::BufferView
{
public:
    FakeBufferView(
        std::shared_ptr<Engine::Graphics::Buffer> buffer,
        std::size_t offset,
        std::size_t size)
        : buffer_(std::move(buffer)), offset_(offset), size_(size)
    {
    }

    std::shared_ptr<Engine::Graphics::Buffer> GetBuffer() const override { return buffer_; }
    std::size_t Offset() const noexcept override { return offset_; }
    std::size_t Size() const noexcept override { return size_; }

private:
    std::shared_ptr<Engine::Graphics::Buffer> buffer_;
    std::size_t offset_;
    std::size_t size_;
};

class FakeBindingSet final : public Engine::Graphics::BindingSet
{
public:
    explicit FakeBindingSet(std::shared_ptr<Engine::Graphics::BindingLayout> layout)
        : layout_(std::move(layout))
    {
    }

    std::shared_ptr<Engine::Graphics::BindingLayout> Layout() const override
    {
        return layout_;
    }

    bool SetBuffer(
        std::uint32_t binding,
        std::uint32_t arrayElement,
        const std::shared_ptr<Engine::Graphics::BufferView> &view,
        Engine::Error &error) override
    {
        if (binding != 0 || arrayElement != 0 || view == nullptr) {
            error.message = "fake buffer binding invalid";
            return false;
        }
        bufferView_ = view;
        return true;
    }

    bool SetTexture(
        std::uint32_t,
        std::uint32_t,
        const std::shared_ptr<Engine::Graphics::TextureView> &,
        Engine::Error &) override
    {
        return false;
    }

    bool SetSampler(
        std::uint32_t,
        std::uint32_t,
        const std::shared_ptr<Engine::Graphics::Sampler> &,
        Engine::Error &) override
    {
        return false;
    }

private:
    std::shared_ptr<Engine::Graphics::BindingLayout> layout_;
    std::shared_ptr<Engine::Graphics::BufferView> bufferView_;
};

class FakeGraphicsPipeline final : public Engine::Graphics::GraphicsPipeline
{
public:
    explicit FakeGraphicsPipeline(const Engine::Graphics::GraphicsPipelineDesc &description)
        : description_(description)
    {
    }

    const Engine::Graphics::GraphicsPipelineDesc &Description() const noexcept override
    {
        return description_;
    }

    std::shared_ptr<Engine::Graphics::BindingLayout> Layout() const override
    {
        return description_.bindingLayout;
    }

private:
    Engine::Graphics::GraphicsPipelineDesc description_;
};

class FakeCommandBuffer final : public Engine::Graphics::CommandBuffer
{
public:
    explicit FakeCommandBuffer(bool copy) : copy_(copy) {}
    bool Reset(Engine::Error &) override { return true; }
    bool IsCopy() const noexcept { return copy_; }

private:
    bool copy_;
};

class FakeSubmission final : public Engine::Graphics::Submission
{
public:
    explicit FakeSubmission(bool complete) : complete_(complete) {}
    bool IsComplete() const override { return complete_; }
    bool Wait(Engine::Error &) override
    {
        complete_ = true;
        return true;
    }
    void Complete() noexcept { complete_ = true; }

private:
    bool complete_;
};

class FakeCopyEncoder final : public Engine::Graphics::CopyEncoder
{
public:
    explicit FakeCopyEncoder(std::shared_ptr<FakeState> state) : state_(std::move(state)) {}

    bool End(Engine::Error &) override
    {
        state_->events.push_back("copy.end");
        return true;
    }

    bool CopyBuffer(
        const std::shared_ptr<Engine::Graphics::Buffer> &source,
        std::size_t sourceOffset,
        const std::shared_ptr<Engine::Graphics::Buffer> &destination,
        std::size_t destinationOffset,
        std::size_t size,
        Engine::Error &error) override
    {
        auto sourceBuffer = std::dynamic_pointer_cast<FakeBuffer>(source);
        auto destinationBuffer = std::dynamic_pointer_cast<FakeBuffer>(destination);
        if (sourceBuffer == nullptr || destinationBuffer == nullptr
            || sourceOffset + size > sourceBuffer->Bytes().size()
            || destinationOffset + size > destinationBuffer->Bytes().size()) {
            error.message = "fake buffer copy out of range";
            return false;
        }
        std::memcpy(
            destinationBuffer->Bytes().data() + destinationOffset,
            sourceBuffer->Bytes().data() + sourceOffset,
            size);
        state_->events.push_back("copy.buffer");
        return true;
    }

    bool CopyTextureToBuffer(
        const std::shared_ptr<Engine::Graphics::Texture> &,
        const std::shared_ptr<Engine::Graphics::Buffer> &destination,
        std::size_t destinationOffset,
        std::uint32_t bytesPerRow,
        Engine::Graphics::Extent3D extent,
        Engine::Error &error) override
    {
        auto destinationBuffer = std::dynamic_pointer_cast<FakeBuffer>(destination);
        const std::size_t size = static_cast<std::size_t>(bytesPerRow) * extent.height;
        if (destinationBuffer == nullptr
            || destinationOffset + size > destinationBuffer->Bytes().size()) {
            error.message = "fake texture copy out of range";
            return false;
        }
        std::memset(destinationBuffer->Bytes().data() + destinationOffset, 0xAB, size);
        state_->events.push_back("copy.texture");
        return true;
    }

private:
    std::shared_ptr<FakeState> state_;
};

class FakeRenderPass final : public Engine::Graphics::RenderPassEncoder
{
public:
    explicit FakeRenderPass(std::shared_ptr<FakeState> state) : state_(std::move(state)) {}

    bool End(Engine::Error &) override
    {
        state_->events.push_back("render.end");
        return true;
    }

    bool SetPipeline(
        const std::shared_ptr<Engine::Graphics::GraphicsPipeline> &,
        Engine::Error &) override
    {
        state_->events.push_back("pipeline");
        return true;
    }

    bool SetBindingSet(
        std::uint32_t index,
        const std::shared_ptr<Engine::Graphics::BindingSet> &,
        Engine::Error &error) override
    {
        if (index != 0) {
            error.message = "fake binding set index invalid";
            return false;
        }
        state_->events.push_back("bindings");
        return true;
    }

    bool SetViewport(const Engine::Graphics::Viewport &viewport, Engine::Error &) override
    {
        state_->viewport = viewport;
        state_->events.push_back("viewport");
        return true;
    }

    bool SetVertexBuffer(
        std::uint32_t slot,
        const std::shared_ptr<Engine::Graphics::Buffer> &buffer,
        std::size_t,
        Engine::Error &error) override
    {
        if (buffer == nullptr) {
            error.message = "fake vertex buffer missing";
            return false;
        }
        state_->boundSlots.push_back(slot);
        return true;
    }

    bool SetIndexBuffer(
        const std::shared_ptr<Engine::Graphics::Buffer> &,
        std::size_t,
        Engine::Graphics::IndexFormat,
        Engine::Error &) override
    {
        return false;
    }

    void Draw(
        std::uint32_t vertexCount,
        std::uint32_t,
        std::uint32_t,
        std::uint32_t) override
    {
        state_->drawVertexCount = vertexCount;
        state_->events.push_back("draw");
    }

    void DrawIndexed(
        std::uint32_t,
        std::uint32_t,
        std::uint32_t,
        std::int32_t,
        std::uint32_t) override
    {
    }

private:
    std::shared_ptr<FakeState> state_;
};

class FakeCommandContext final : public Engine::Graphics::CommandContext
{
public:
    explicit FakeCommandContext(std::shared_ptr<FakeState> state) : state_(std::move(state)) {}

    std::unique_ptr<Engine::Graphics::RenderPassEncoder> BeginRenderPass(
        const Engine::Graphics::RenderPassDesc &description,
        Engine::Error &) override
    {
        isCopy_ = false;
        state_->events.push_back("render.begin");
        check(description.colorAttachments.size() == 1, "draw must have one color attachment");
        return std::unique_ptr<Engine::Graphics::RenderPassEncoder>(new FakeRenderPass(state_));
    }

    std::unique_ptr<Engine::Graphics::ComputeEncoder> BeginCompute(Engine::Error &) override
    {
        return nullptr;
    }

    std::unique_ptr<Engine::Graphics::CopyEncoder> BeginCopy(Engine::Error &) override
    {
        isCopy_ = true;
        state_->events.push_back("copy.begin");
        return std::unique_ptr<Engine::Graphics::CopyEncoder>(new FakeCopyEncoder(state_));
    }

    bool Barrier(
        const std::shared_ptr<Engine::Graphics::Resource> &,
        const Engine::Graphics::ResourceBarrierDesc &description,
        Engine::Error &) override
    {
        state_->barrierTargets.push_back(description.after);
        state_->events.push_back("barrier");
        return true;
    }

    std::shared_ptr<Engine::Graphics::CommandBuffer> Finish(Engine::Error &) override
    {
        state_->events.push_back("finish");
        return std::make_shared<FakeCommandBuffer>(isCopy_);
    }

private:
    std::shared_ptr<FakeState> state_;
    bool isCopy_ = false;
};

class FakeQueue final : public Engine::Graphics::CommandQueue
{
public:
    explicit FakeQueue(std::shared_ptr<FakeState> state) : state_(std::move(state)) {}

    Engine::Graphics::QueueType Type() const noexcept override
    {
        return Engine::Graphics::QueueType::Graphics;
    }

    std::shared_ptr<Engine::Graphics::CommandContext> CreateContext(
        Engine::Error &) override
    {
        return std::make_shared<FakeCommandContext>(state_);
    }

    std::shared_ptr<Engine::Graphics::Submission> Submit(
        const std::shared_ptr<Engine::Graphics::CommandBuffer> &commandBuffer,
        const std::vector<std::shared_ptr<Engine::Graphics::Submission>> &waitFor,
        const std::vector<std::shared_ptr<Engine::Graphics::Fence>> &,
        Engine::Error &error) override
    {
        auto fakeCommandBuffer = std::dynamic_pointer_cast<FakeCommandBuffer>(commandBuffer);
        if (fakeCommandBuffer == nullptr) {
            error.message = "fake command buffer missing";
            return nullptr;
        }
        state_->waitCounts.push_back(waitFor.size());
        if (fakeCommandBuffer->IsCopy()) {
            state_->events.push_back("submit.copy");
            auto submission = std::make_shared<FakeSubmission>(false);
            state_->uploadSubmission = submission;
            return submission;
        }
        state_->events.push_back("submit.draw");
        return std::make_shared<FakeSubmission>(true);
    }

private:
    std::shared_ptr<FakeState> state_;
};

class FakeDevice final : public Engine::Graphics::Device
{
public:
    explicit FakeDevice(std::shared_ptr<FakeState> state)
        : state_(std::move(state)), queue_(std::make_shared<FakeQueue>(state_))
    {
    }

    const Engine::Graphics::DeviceCapabilities &Capabilities() const noexcept override
    {
        return capabilities_;
    }

    std::shared_ptr<Engine::Graphics::Buffer> CreateBuffer(
        const Engine::Graphics::BufferDesc &description,
        Engine::Error &) override
    {
        auto buffer = std::make_shared<FakeBuffer>(description);
        state_->buffers.push_back(buffer);
        return buffer;
    }

    std::shared_ptr<Engine::Graphics::Texture> CreateTexture(
        const Engine::Graphics::TextureDesc &description,
        Engine::Error &) override
    {
        return std::make_shared<FakeTexture>(description);
    }

    std::shared_ptr<Engine::Graphics::Shader> CreateShader(
        const Engine::Graphics::ShaderDesc &,
        Engine::Error &) override
    {
        return nullptr;
    }

    std::shared_ptr<Engine::Graphics::BufferView> CreateBufferView(
        const std::shared_ptr<Engine::Graphics::Buffer> &buffer,
        std::size_t offset,
        std::size_t size,
        Engine::Error &) override
    {
        return std::make_shared<FakeBufferView>(buffer, offset, size);
    }

    std::shared_ptr<Engine::Graphics::TextureView> CreateTextureView(
        const std::shared_ptr<Engine::Graphics::Texture> &texture,
        const Engine::Graphics::ResourceRange &,
        Engine::Graphics::TextureFormat,
        Engine::Error &) override
    {
        return std::make_shared<FakeTextureView>(texture);
    }

    std::shared_ptr<Engine::Graphics::Sampler> CreateSampler(
        const Engine::Graphics::SamplerDesc &,
        Engine::Error &) override
    {
        return nullptr;
    }

    std::shared_ptr<Engine::Graphics::BindingLayout> CreateBindingLayout(
        const Engine::Graphics::BindingLayoutDesc &description,
        Engine::Error &) override
    {
        return std::make_shared<FakeBindingLayout>(description);
    }

    std::shared_ptr<Engine::Graphics::BindingSet> CreateBindingSet(
        const std::shared_ptr<Engine::Graphics::BindingLayout> &layout,
        Engine::Error &) override
    {
        return std::make_shared<FakeBindingSet>(layout);
    }

    std::shared_ptr<Engine::Graphics::GraphicsPipeline> CreateGraphicsPipeline(
        const Engine::Graphics::GraphicsPipelineDesc &description,
        Engine::Error &) override
    {
        state_->pipeline = std::make_shared<FakeGraphicsPipeline>(description);
        return state_->pipeline;
    }

    std::shared_ptr<Engine::Graphics::Swapchain> CreateSwapchain(
        Engine::Graphics::PresentationSurface &,
        const Engine::Graphics::SwapchainDesc &description,
        Engine::Error &) override
    {
        auto swapchain = std::make_shared<FakeSwapchain>(state_, description);
        state_->swapchain = swapchain;
        return swapchain;
    }

    std::shared_ptr<Engine::Graphics::ComputePipeline> CreateComputePipeline(
        const Engine::Graphics::ComputePipelineDesc &,
        Engine::Error &) override
    {
        return nullptr;
    }

    std::shared_ptr<Engine::Graphics::CommandQueue> GetQueue(
        Engine::Graphics::QueueType type) override
    {
        return type == Engine::Graphics::QueueType::Graphics ? queue_ : nullptr;
    }

    std::shared_ptr<Engine::Graphics::Fence> CreateFence(Engine::Error &) override
    {
        return nullptr;
    }

    std::shared_ptr<Engine::Graphics::MemoryAllocator> GetMemoryAllocator() override
    {
        return nullptr;
    }

private:
    std::shared_ptr<FakeState> state_;
    std::shared_ptr<FakeQueue> queue_;
    Engine::Graphics::DeviceCapabilities capabilities_;
};

class FakeAdapter final : public Engine::Graphics::Adapter
{
public:
    FakeAdapter(
        std::shared_ptr<FakeState> state,
        std::shared_ptr<FakeDevice> device,
        bool graphicsQueue)
        : state_(std::move(state)), device_(std::move(device))
    {
        if (graphicsQueue) {
            capabilities_.queueTypes.push_back(Engine::Graphics::QueueType::Graphics);
        }
    }

    const Engine::Graphics::DeviceCapabilities &Capabilities() const noexcept override
    {
        return capabilities_;
    }

    std::shared_ptr<Engine::Graphics::Device> CreateDevice(Engine::Error &) override
    {
        ++state_->deviceCreateCount;
        return device_;
    }

private:
    std::shared_ptr<FakeState> state_;
    std::shared_ptr<FakeDevice> device_;
    Engine::Graphics::DeviceCapabilities capabilities_;
};

class FakeGraphicsInstance final : public Engine::Graphics::GraphicsInstance
{
public:
    explicit FakeGraphicsInstance(std::vector<std::shared_ptr<Engine::Graphics::Adapter>> adapters)
        : adapters_(std::move(adapters))
    {
    }

    bool EnumerateAdapters(
        std::vector<std::shared_ptr<Engine::Graphics::Adapter>> &adapters,
        Engine::Error &) override
    {
        adapters = adapters_;
        return true;
    }

private:
    std::vector<std::shared_ptr<Engine::Graphics::Adapter>> adapters_;
};

class FakeShaderLibrary final : public Engine::ShaderLibrary
{
public:
    bool RequestGraphicsShaders(
        Engine::Graphics::Device &,
        Engine::ShaderProgramType type,
        const std::string &name,
        Engine::GraphicsShaderPair &shaders,
        Engine::Error &error) override
    {
        if (type != Engine::ShaderProgramType::Mesh || name != "uv-triangle") {
            error.message = "unexpected shader request";
            return false;
        }
        shaders.vertex = std::make_shared<FakeShader>(Engine::Graphics::ShaderStage::Vertex);
        shaders.fragment = std::make_shared<FakeShader>(Engine::Graphics::ShaderStage::Fragment);
        return true;
    }
};

class FakeSurface final : public Engine::Surface
{
public:
    Engine::Size DrawableSize() const noexcept override { return size_; }

    bool Start(const Callbacks &callbacks, Engine::Error &error) override
    {
        callbacks_ = callbacks;
        if (!startSucceeds) {
            error.message = "fake surface startup failed";
            return false;
        }
        started_ = true;
        return true;
    }

    void Stop() noexcept override
    {
        started_ = false;
        ++stopCount;
    }

    bool Draw(Engine::Error &error)
    {
        return started_ && callbacks_.Draw(callbacks_.context, error);
    }

    bool Resize(Engine::Size size, Engine::Error &error)
    {
        size_ = size;
        return started_ && callbacks_.Resize(callbacks_.context, size, error);
    }

private:
    Engine::Size size_{640, 480};
    Callbacks callbacks_{};
    bool started_ = false;

public:
    bool startSucceeds = true;
    std::uint32_t stopCount = 0;
};

void surfaceStartupFailureCleansUpEngineResources()
{
    auto state = std::make_shared<FakeState>();
    auto device = std::make_shared<FakeDevice>(state);
    std::vector<std::shared_ptr<Engine::Graphics::Adapter>> adapters = {
        std::make_shared<FakeAdapter>(state, device, true),
    };
    auto graphicsInstance = std::make_shared<FakeGraphicsInstance>(std::move(adapters));
    auto shaderLibrary = std::make_shared<FakeShaderLibrary>();
    std::unique_ptr<Engine::Engine> engine =
        Engine::EngineFactory::create(graphicsInstance, shaderLibrary);
    FakeSurface surface;
    surface.startSucceeds = false;
    Engine::Error error;

    check(!engine->Initialize(surface, [](const Engine::Error &) {}, error),
        "surface startup failure should fail initialization");
    check(error.message == "fake surface startup failed",
        "surface startup error should remain visible");
    check(surface.stopCount == 1, "failed surface startup should stop the surface once");
    engine->Shutdown();
    check(surface.stopCount == 1, "shutdown should not stop an already stopped surface");
}

void engineRecordsAThreeVertexUvMeshDraw()
{
    auto state = std::make_shared<FakeState>();
    auto device = std::make_shared<FakeDevice>(state);
    std::vector<std::shared_ptr<Engine::Graphics::Adapter>> adapters = {
        std::make_shared<FakeAdapter>(state, device, false),
        std::make_shared<FakeAdapter>(state, device, true),
    };
    auto graphicsInstance = std::make_shared<FakeGraphicsInstance>(std::move(adapters));
    auto shaderLibrary = std::make_shared<FakeShaderLibrary>();
    std::unique_ptr<Engine::Engine> engine =
        Engine::EngineFactory::create(graphicsInstance, shaderLibrary);
    FakeSurface surface;
    Engine::Error error;
    int errorReports = 0;

    check(engine->Initialize(surface, [&errorReports](const Engine::Error &) {
        ++errorReports;
    }, error), "engine initialization should succeed");
    check(state->deviceCreateCount == 1,
        "engine should create a device from the first graphics-capable adapter");
    check(state->pipeline != nullptr, "engine should create a graphics pipeline");
    check(state->pipeline->Description().bindingLayout != nullptr,
        "graphics pipeline should identify its binding layout");
    check(state->pipeline->Description().topology
            == Engine::Graphics::PrimitiveTopology::TriangleList,
        "triangle pipeline should use triangle-list topology");
    check(state->pipeline->Description().vertexAttributes.size() == 2,
        "triangle pipeline should consume position and UV attributes");

    std::shared_ptr<FakeBuffer> transformBuffer;
    std::shared_ptr<FakeBuffer> positionBuffer;
    std::shared_ptr<FakeBuffer> uvBuffer;
    for (const std::shared_ptr<FakeBuffer> &buffer : state->buffers) {
        if (buffer->Description().usage == Engine::Graphics::BufferUsage::Uniform) {
            transformBuffer = buffer;
        } else if (buffer->Description().usage
            == (Engine::Graphics::BufferUsage::Vertex
                | Engine::Graphics::BufferUsage::CopyDestination)) {
            if (positionBuffer == nullptr) {
                positionBuffer = buffer;
            } else {
                uvBuffer = buffer;
            }
        }
    }
    check(transformBuffer != nullptr && positionBuffer != nullptr && uvBuffer != nullptr,
        "engine should allocate its transform block and two GPU vertex streams");
    float transforms[48] = {};
    std::memcpy(transforms, transformBuffer->Bytes().data(), sizeof(transforms));
    for (std::size_t matrix = 0; matrix < 3; ++matrix) {
        for (std::size_t element = 0; element < 16; ++element) {
            const float expected = element % 5 == 0 ? 1.0f : 0.0f;
            check(transforms[matrix * 16 + element] == expected,
                "world, view, and projection identities should be stored in the uniform buffer");
        }
    }
    float positions[9] = {};
    float uvs[6] = {};
    std::memcpy(positions, positionBuffer->Bytes().data(), sizeof(positions));
    std::memcpy(uvs, uvBuffer->Bytes().data(), sizeof(uvs));
    check(std::fabs(positions[4] - 0.57735027f) < 0.0001f,
        "triangle position stream should contain the equilateral apex");
    check(uvs[0] == 0.0f && uvs[1] == 1.0f
            && uvs[2] == 0.5f && uvs[3] == 0.0f
            && uvs[4] == 1.0f && uvs[5] == 1.0f,
        "UV stream should cover all three triangle corners");

    check(surface.Draw(error), "surface draw callback should record the mesh");
    check(state->drawVertexCount == 3, "draw should submit exactly three vertices");
    check(state->boundSlots == std::vector<std::uint32_t>({0, 1}),
        "draw should bind both vertex streams");
    check(state->waitCounts == std::vector<std::size_t>({0, 1}),
        "draw submission should depend on the asynchronous mesh upload");
    check(std::find(
            state->barrierTargets.begin(), state->barrierTargets.end(),
            Engine::Graphics::ResourceUsage::UniformBuffer) != state->barrierTargets.end(),
        "draw should transition transform data to uniform-buffer use");
    check(state->presentCount == 1, "draw should present its swapchain image");
    check(state->viewport.width == 640.0f && state->viewport.height == 480.0f,
        "draw should set a viewport matching the drawable extent");

    check(surface.Resize({1024, 768}, error), "surface resize should reach the swapchain");
    check(surface.Draw(error), "draw should continue after swapchain resize");
    check(state->viewport.width == 1024.0f && state->viewport.height == 768.0f,
        "resized draw should update its viewport");

    auto upload = state->uploadSubmission.lock();
    check(upload != nullptr, "mesh upload submission should be retained");
    upload->Complete();
    check(surface.Draw(error), "draw should collect completed uploads");
    check(state->waitCounts.back() == 0,
        "completed mesh upload should no longer be a submission dependency");

    check(surface.Resize({0, 0}, error), "minimize resize should succeed");
    const std::uint32_t acquiredBeforeMinimize = state->acquireCount;
    check(surface.Draw(error), "minimized surface should skip drawing");
    check(state->acquireCount == acquiredBeforeMinimize,
        "minimized surface should not acquire a swapchain image");

    check(surface.Resize({1024, 768}, error), "restoring surface size should succeed");
    state->presentSucceeds = false;
    check(!surface.Draw(error), "presentation errors should fail the draw callback");
    check(error.message == "fake present failed", "present failure should remain visible");
    check(errorReports == 1, "runtime draw failure should be reported once");
    check(!surface.Draw(error), "draw should stop after a reported runtime failure");
    check(errorReports == 1, "failed engine should not report repeated draw errors");
    engine->Shutdown();
}

void headlessEngineRendersToAnOffscreenTextureAndCapturesIt()
{
    auto state = std::make_shared<FakeState>();
    auto device = std::make_shared<FakeDevice>(state);
    std::vector<std::shared_ptr<Engine::Graphics::Adapter>> adapters = {
        std::make_shared<FakeAdapter>(state, device, true),
    };
    auto graphicsInstance = std::make_shared<FakeGraphicsInstance>(std::move(adapters));
    std::unique_ptr<Engine::Engine> engine = Engine::EngineFactory::create(
        graphicsInstance, std::make_shared<FakeShaderLibrary>());
    Engine::Error error;
    Engine::Graphics::CapturedImage image;

    check(!engine->RenderHeadlessFrame(error), "headless render requires headless initialization");
    check(engine->InitializeHeadless({64, 32}, [](const Engine::Error &) {}, error),
        "headless initialization should succeed");
    check(!engine->CaptureHeadlessImage(image), "no image exists before the first frame");
    check(engine->RenderHeadlessFrame(error), "headless frame should render");
    check(state->drawVertexCount == 3, "headless frame should draw the mesh");
    check(state->presentCount == 0, "headless frame should not use a window swapchain");
    check(std::find(state->events.begin(), state->events.end(), "copy.texture")
            != state->events.end(),
        "headless present should copy the frame into a readback buffer");
    check(engine->CaptureHeadlessImage(image), "headless frame should be capturable");
    check(image.width == 64 && image.height == 32
            && image.pixels.size() == 64u * 32u * 4u && image.pixels[0] == 0xAB,
        "captured image should carry the readback contents");
    engine->Shutdown();
    check(!engine->CaptureHeadlessImage(image), "shutdown should release the capture target");
}

} // namespace

int main()
{
    headlessEngineRendersToAnOffscreenTextureAndCapturesIt();
    surfaceStartupFailureCleansUpEngineResources();
    engineRecordsAThreeVertexUvMeshDraw();
}
