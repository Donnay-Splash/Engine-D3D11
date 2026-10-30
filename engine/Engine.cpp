#include "Engine.h"

#include "Core/Math.h"
#include "Core/MeshMaker.h"
#include "EngineFactory.h"
#include "Graphics/Adapter.h"
#include "Graphics/BindingLayout.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/BufferView.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandContext.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/Device.h"
#include "Graphics/GraphicsInstance.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Shader.h"
#include "Graphics/Swapchain.h"
#include "Graphics/SwapchainImage.h"
#include "Graphics/TextureView.h"
#include "ShaderLibrary.h"

#include <algorithm>
#include <cstring>
#include <cstddef>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace Engine {
namespace {

struct alignas(16) MeshTransforms
{
    Maths::Matrix4 world;
    Maths::Matrix4 view;
    Maths::Matrix4 projection;
};

static_assert(std::is_standard_layout<MeshTransforms>::value,
    "Transform uniforms must have standard layout.");
static_assert(sizeof(MeshTransforms) == sizeof(float) * 48,
    "Transform uniforms must contain three packed matrices.");
static_assert(offsetof(MeshTransforms, view) == sizeof(float) * 16,
    "View transform must immediately follow world transform.");
static_assert(offsetof(MeshTransforms, projection) == sizeof(float) * 32,
    "Projection transform must immediately follow view transform.");

} // namespace

struct Engine::ErrorRelay
{
    explicit ErrorRelay(const ErrorHandler &errorHandler)
        : handler(errorHandler), failed(false)
    {
    }

    bool notify(const Error &error)
    {
        ErrorHandler callback;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (failed) {
                return false;
            }
            failed = true;
            callback = handler;
        }
        if (callback) {
            callback(error);
        }
        return true;
    }

    bool hasFailed()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return failed;
    }

    void deactivate()
    {
        std::lock_guard<std::mutex> lock(mutex);
        failed = true;
        handler = ErrorHandler();
    }

    std::mutex mutex;
    ErrorHandler handler;
    bool failed;
};

Engine::Engine(
    std::shared_ptr<Graphics::GraphicsInstance> graphicsInstance,
    std::shared_ptr<ShaderLibrary> shaderLibrary,
    std::shared_ptr<Graphics::Adapter> preferredAdapter)
    : m_GraphicsInstance(std::move(graphicsInstance)),
      m_ShaderLibrary(std::move(shaderLibrary)),
      m_PreferredAdapter(std::move(preferredAdapter)),
      m_Surface(nullptr),
      m_Size({0, 0}),
      m_ErrorRelay(),
      m_Initialized(false),
      m_SurfaceStarted(false),
      m_TransformBufferPrepared(false)
{
}

Engine::~Engine()
{
    Shutdown();
}

bool Engine::Initialize(Surface &surface, ErrorHandler errorHandler, Error &error)
{
    if (m_Initialized) {
        error.message = "The engine is already initialized.";
        return false;
    }
    if (m_GraphicsInstance == nullptr || m_ShaderLibrary == nullptr) {
        error.message = "The engine requires a graphics instance and shader library.";
        return false;
    }

    m_Surface = &surface;
    m_ErrorRelay = std::shared_ptr<ErrorRelay>(new ErrorRelay(errorHandler));
    if (!InitializeGraphics(surface, error)) {
        Shutdown();
        return false;
    }

    m_Initialized = true;
    m_SurfaceStarted = true;
    const Surface::Callbacks callbacks = {
        this,
        &Engine::Draw,
        &Engine::Resize,
        &Engine::SurfaceFailure,
    };
    if (!surface.Start(callbacks, error)) {
        if (error.message.empty()) {
            error.message = "The render surface failed to start.";
        }
        surface.Stop();
        m_SurfaceStarted = false;
        Shutdown();
        return false;
    }
    return true;
}

bool Engine::InitializeGraphics(Surface &surface, Error &error)
{
    std::vector<std::shared_ptr<Graphics::Adapter>> adapters;
    if (!m_GraphicsInstance->EnumerateAdapters(adapters, error)) {
        if (error.message.empty()) {
            error.message = "Failed to enumerate graphics adapters.";
        }
        return false;
    }

    std::shared_ptr<Graphics::Adapter> selectedAdapter = m_PreferredAdapter;
    if (selectedAdapter != nullptr) {
        bool isEnumeratedAdapter = false;
        for (const std::shared_ptr<Graphics::Adapter> &adapter : adapters) {
            if (adapter == selectedAdapter) {
                isEnumeratedAdapter = true;
                break;
            }
        }
        if (!isEnumeratedAdapter) {
            error.message = "The selected graphics adapter was not enumerated by this instance.";
            return false;
        }
    } else {
        for (const std::shared_ptr<Graphics::Adapter> &adapter : adapters) {
            if (adapter == nullptr) {
                continue;
            }
            const std::vector<Graphics::QueueType> &queueTypes =
                adapter->Capabilities().queueTypes;
            for (Graphics::QueueType queueType : queueTypes) {
                if (queueType == Graphics::QueueType::Graphics) {
                    selectedAdapter = adapter;
                    break;
                }
            }
            if (selectedAdapter != nullptr) {
                break;
            }
        }
    }

    if (selectedAdapter != nullptr) {
        const std::vector<Graphics::QueueType> &queueTypes =
            selectedAdapter->Capabilities().queueTypes;
        const bool supportsGraphics = std::find(
            queueTypes.begin(), queueTypes.end(), Graphics::QueueType::Graphics)
            != queueTypes.end();
        if (!supportsGraphics) {
            error.message = "The selected graphics adapter has no graphics queue.";
            return false;
        }
    }
    if (selectedAdapter == nullptr) {
        error.message = "No graphics adapter with a graphics queue is available.";
        return false;
    }

    m_Device = selectedAdapter->CreateDevice(error);
    if (m_Device == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a graphics device.";
        }
        return false;
    }
    m_GraphicsQueue = m_Device->GetQueue(Graphics::QueueType::Graphics);
    if (m_GraphicsQueue == nullptr) {
        error.message = "The graphics device has no graphics command queue.";
        return false;
    }

    m_Size = surface.DrawableSize();
    if (m_Size.width == 0 || m_Size.height == 0) {
        error.message = "The render surface has no drawable extent.";
        return false;
    }
    Graphics::SwapchainDesc swapchainDescription;
    swapchainDescription.extent = {m_Size.width, m_Size.height, 1};
    m_Swapchain = m_Device->CreateSwapchain(surface, swapchainDescription, error);
    if (m_Swapchain == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a swapchain for the render surface.";
        }
        return false;
    }

    GraphicsShaderPair shaders;
    if (!m_ShaderLibrary->RequestGraphicsShaders(
            *m_Device, ShaderProgramType::Mesh, "uv-triangle", shaders, error)) {
        if (error.message.empty()) {
            error.message = "Failed to request the UV mesh shader pair.";
        }
        return false;
    }
    if (shaders.vertex == nullptr || shaders.fragment == nullptr
        || shaders.vertex->Stage() != Graphics::ShaderStage::Vertex
        || shaders.fragment->Stage() != Graphics::ShaderStage::Fragment) {
        error.message = "The shader library returned an invalid mesh shader pair.";
        return false;
    }

    Graphics::BindingLayoutDesc bindingLayoutDescription;
    bindingLayoutDescription.bindings.push_back({
        0,
        Graphics::BindingType::UniformBuffer,
        1,
        Graphics::ShaderStage::Vertex,
    });
    std::shared_ptr<Graphics::BindingLayout> bindingLayout =
        m_Device->CreateBindingLayout(bindingLayoutDescription, error);
    if (bindingLayout == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform binding layout.";
        }
        return false;
    }
    m_Bindings = m_Device->CreateBindingSet(bindingLayout, error);
    if (m_Bindings == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform binding set.";
        }
        return false;
    }

    Graphics::BufferDesc transformDescription;
    transformDescription.size = sizeof(MeshTransforms);
    transformDescription.usage = Graphics::BufferUsage::Uniform;
    transformDescription.memoryType = Graphics::MemoryType::Upload;
    m_TransformBuffer = m_Device->CreateBuffer(transformDescription, error);
    if (m_TransformBuffer == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the transform uniform buffer.";
        }
        return false;
    }
    const MeshTransforms transforms;
    void *transformData = nullptr;
    if (!m_TransformBuffer->Map(transformData, error)) {
        return false;
    }
    if (transformData == nullptr) {
        m_TransformBuffer->Unmap();
        error.message = "The transform uniform buffer returned no mapped memory.";
        return false;
    }
    std::memcpy(transformData, &transforms, sizeof(transforms));
    m_TransformBuffer->Unmap();

    m_TransformBufferView = m_Device->CreateBufferView(
        m_TransformBuffer, 0, sizeof(MeshTransforms), error);
    if (m_TransformBufferView == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a view of the transform uniform buffer.";
        }
        return false;
    }
    if (!m_Bindings->SetBuffer(0, 0, m_TransformBufferView, error)) {
        if (error.message.empty()) {
            error.message = "Failed to bind the transform uniform buffer.";
        }
        return false;
    }

    if (!MeshMaker::CreateUniformTriangle(*m_Device, m_Mesh, error)) {
        if (error.message.empty()) {
            error.message = "Failed to create the uniform triangle mesh.";
        }
        return false;
    }

    Graphics::GraphicsPipelineDesc pipelineDescription;
    pipelineDescription.shaders = {shaders.vertex, shaders.fragment};
    pipelineDescription.bindingLayout = bindingLayout;
    pipelineDescription.vertexAttributes = {
        {0, 0, Graphics::VertexFormat::Float3, 0},
        {1, 1, Graphics::VertexFormat::Float2, 0},
    };
    pipelineDescription.vertexBuffers = {
        {0, static_cast<std::uint32_t>(sizeof(float) * 3)},
        {1, static_cast<std::uint32_t>(sizeof(float) * 2)},
    };
    pipelineDescription.topology = Graphics::PrimitiveTopology::TriangleList;
    pipelineDescription.cullMode = Graphics::CullMode::None;
    pipelineDescription.colorFormats = {m_Swapchain->Description().format};
    m_GraphicsPipeline = m_Device->CreateGraphicsPipeline(pipelineDescription, error);
    if (m_GraphicsPipeline == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the mesh graphics pipeline.";
        }
        return false;
    }
    return true;
}

void Engine::Shutdown() noexcept
{
    if (m_ErrorRelay != nullptr) {
        m_ErrorRelay->deactivate();
    }
    if (m_SurfaceStarted && m_Surface != nullptr) {
        m_Surface->Stop();
        m_SurfaceStarted = false;
    }
    m_Initialized = false;
    m_GraphicsPipeline.reset();
    m_Bindings.reset();
    m_TransformBufferView.reset();
    m_TransformBuffer.reset();
    m_Mesh = Mesh();
    m_Swapchain.reset();
    m_GraphicsQueue.reset();
    m_Device.reset();
    m_Size = {0, 0};
    m_TransformBufferPrepared = false;
    m_Surface = nullptr;
    m_ErrorRelay.reset();
}

bool Engine::Draw(void *context, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->m_Initialized
        || engine->m_ErrorRelay == nullptr || engine->m_ErrorRelay->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    return engine->DrawFrame(error);
}

bool Engine::DrawFrame(Error &error)
{
    const auto fail = [this, &error](const char *fallback) {
        if (error.message.empty()) {
            error.message = fallback;
        }
        ReportFailure(error);
        return false;
    };
    if (m_Size.width == 0 || m_Size.height == 0) {
        return true;
    }

    std::vector<std::shared_ptr<Graphics::Submission>> waitFor;
    if (!m_Mesh.CollectPendingUploads(waitFor, error)) {
        return fail("A mesh vertex upload failed.");
    }
    std::shared_ptr<Graphics::SwapchainImage> image =
        m_Swapchain->AcquireNextImage(error);
    if (image == nullptr) {
        return fail("Failed to acquire a swapchain image.");
    }
    std::shared_ptr<Graphics::TextureView> colorView = image->View();
    if (colorView == nullptr) {
        error.message = "The acquired swapchain image has no texture view.";
        return fail("The acquired swapchain image has no texture view.");
    }

    Graphics::RenderAttachmentDesc colorAttachment;
    colorAttachment.view = colorView;
    colorAttachment.loadAction = Graphics::LoadAction::Clear;
    colorAttachment.storeAction = Graphics::StoreAction::Store;
    colorAttachment.clearColor[0] = 0.08f;
    colorAttachment.clearColor[1] = 0.09f;
    colorAttachment.clearColor[2] = 0.12f;
    colorAttachment.clearColor[3] = 1.0f;
    Graphics::RenderPassDesc renderPassDescription;
    renderPassDescription.colorAttachments.push_back(colorAttachment);
    renderPassDescription.extent = {m_Size.width, m_Size.height, 1};

    std::shared_ptr<Graphics::CommandContext> context =
        m_GraphicsQueue->CreateContext(error);
    if (context == nullptr) {
        return fail("Failed to create a graphics command context.");
    }
    if (!m_TransformBufferPrepared) {
        Graphics::ResourceBarrierDesc transformBarrier;
        transformBarrier.before = Graphics::ResourceUsage::Undefined;
        transformBarrier.after = Graphics::ResourceUsage::UniformBuffer;
        if (!context->Barrier(m_TransformBuffer, transformBarrier, error)) {
            return fail("Failed to prepare the transform uniform buffer.");
        }
    }
    std::unique_ptr<Graphics::RenderPassEncoder> renderPass =
        context->BeginRenderPass(renderPassDescription, error);
    if (renderPass == nullptr) {
        return fail("Failed to begin the mesh render pass.");
    }
    if (!renderPass->SetPipeline(m_GraphicsPipeline, error)
        || !renderPass->SetBindingSet(0, m_Bindings, error)
        || !renderPass->SetViewport({
            0.0f, 0.0f,
            static_cast<float>(m_Size.width),
            static_cast<float>(m_Size.height),
            0.0f, 1.0f,
        }, error)) {
        return fail("Failed to configure the mesh render pass.");
    }
    for (const VertexStream &stream : m_Mesh.VertexStreams()) {
        if (!renderPass->SetVertexBuffer(stream.slot, stream.buffer, 0, error)) {
            return fail("Failed to bind a mesh vertex stream.");
        }
    }
    renderPass->Draw(m_Mesh.VertexCount());
    if (!renderPass->End(error)) {
        return fail("Failed to end the mesh render pass.");
    }

    std::shared_ptr<Graphics::CommandBuffer> commandBuffer = context->Finish(error);
    if (commandBuffer == nullptr) {
        return fail("Failed to finish the mesh draw command buffer.");
    }
    std::shared_ptr<Graphics::Submission> submission =
        m_GraphicsQueue->Submit(commandBuffer, waitFor, {}, error);
    if (submission == nullptr) {
        return fail("Failed to submit the mesh draw.");
    }
    m_TransformBufferPrepared = true;
    if (!m_Swapchain->Present(image, submission, error)) {
        return fail("Failed to present the rendered mesh.");
    }
    return true;
}

bool Engine::Resize(void *context, Size size, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->m_Initialized
        || engine->m_ErrorRelay == nullptr || engine->m_ErrorRelay->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    if (size.width == 0 || size.height == 0) {
        engine->m_Size = size;
        return true;
    }
    const Graphics::Extent3D extent = {size.width, size.height, 1};
    if (!engine->m_Swapchain->Resize(extent, error)) {
        if (error.message.empty()) {
            error.message = "The graphics swapchain failed to resize.";
        }
        engine->ReportFailure(error);
        return false;
    }
    engine->m_Size = size;
    return true;
}

void Engine::SurfaceFailure(void *context, const Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine != nullptr) {
        engine->ReportFailure(error);
    }
}

bool Engine::ReportFailure(const Error &error)
{
    return m_ErrorRelay != nullptr && m_ErrorRelay->notify(error);
}

std::unique_ptr<Engine> EngineFactory::create(
    std::shared_ptr<Graphics::GraphicsInstance> graphicsInstance,
    std::shared_ptr<ShaderLibrary> shaderLibrary,
    std::shared_ptr<Graphics::Adapter> preferredAdapter)
{
    return std::unique_ptr<Engine>(
        new Engine(
            std::move(graphicsInstance),
            std::move(shaderLibrary),
            std::move(preferredAdapter)));
}

} // namespace Engine
