#include "Engine.h"

#include "Core/Math.h"
#include "Core/MeshMaker.h"
#include "Core/Model.h"
#include "Core/ModelInstance.h"
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
#include "Graphics/SwapchainFactory.h"
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

// Deliberately loud so a cleared-but-undrawn frame is obvious in captured output.
constexpr float kClearColor[4] = {1.0f, 0.0f, 0.8f, 1.0f};

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
      m_Config(),
      m_Surface(nullptr),
      m_Size({0, 0}),
      m_ErrorRelay(),
      m_Initialized(false),
      m_SurfaceStarted(false)
{
}

Engine::~Engine()
{
    Shutdown();
}

bool Engine::Initialize(
    Surface &surface, const EngineConfig &config, ErrorHandler errorHandler, Error &error)
{
    if (m_Initialized) {
        error.message = "The engine is already initialized.";
        return false;
    }
    if (m_GraphicsInstance == nullptr || m_ShaderLibrary == nullptr) {
        error.message = "The engine requires a graphics instance and shader library.";
        return false;
    }

    m_Config = config;
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

bool Engine::CaptureImage(Graphics::CapturedImage &image) const
{
    return m_Swapchain != nullptr && m_Swapchain->CaptureLatestImage(image);
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
        error.message = "The render target has no drawable extent.";
        return false;
    }
    Graphics::SwapchainDesc swapchainDescription;
    swapchainDescription.extent = {m_Size.width, m_Size.height, 1};
    m_Swapchain = Graphics::CreateSwapchain(
        m_Device, surface, swapchainDescription, m_Config.swapchainMode, error);
    if (m_Swapchain == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a swapchain for the render target.";
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

    if (!m_SceneRenderer.Initialize(
            m_Device, shaders, m_Swapchain->Description().format, error)) {
        if (error.message.empty()) {
            error.message = "Failed to initialize the scene renderer.";
        }
        return false;
    }

    auto mesh = std::make_shared<Mesh>();
    if (!MeshMaker::CreateUniformTriangle(*m_Device, *mesh, error)) {
        if (error.message.empty()) {
            error.message = "Failed to create the uniform triangle mesh.";
        }
        return false;
    }
    auto model = std::make_shared<Model>();
    if (!model->AddPart(
            {std::move(mesh), Material::CreateFlatColor("default", {1.0f, 1.0f, 1.0f, 1.0f})},
            error)) {
        return false;
    }
    m_Scene.Add(std::make_shared<ModelInstance>(std::move(model)));
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
    m_Scene.Clear();
    m_SceneRenderer.Shutdown();
    m_Swapchain.reset();
    m_GraphicsQueue.reset();
    m_Device.reset();
    m_Size = {0, 0};
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
    if (!m_SceneRenderer.CollectPendingUploads(m_Scene, waitFor, error)) {
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
    colorAttachment.clearColor[0] = kClearColor[0];
    colorAttachment.clearColor[1] = kClearColor[1];
    colorAttachment.clearColor[2] = kClearColor[2];
    colorAttachment.clearColor[3] = kClearColor[3];
    Graphics::RenderPassDesc renderPassDescription;
    renderPassDescription.colorAttachments.push_back(colorAttachment);
    renderPassDescription.extent = {m_Size.width, m_Size.height, 1};

    std::shared_ptr<Graphics::CommandContext> context =
        m_GraphicsQueue->CreateContext(error);
    if (context == nullptr) {
        return fail("Failed to create a graphics command context.");
    }
    if (!m_SceneRenderer.Prepare(m_Scene, *context, error)) {
        return fail("Failed to prepare the scene draws.");
    }
    std::unique_ptr<Graphics::RenderPassEncoder> renderPass =
        context->BeginRenderPass(renderPassDescription, error);
    if (renderPass == nullptr) {
        return fail("Failed to begin the mesh render pass.");
    }
    if (!renderPass->SetViewport({
            0.0f, 0.0f,
            static_cast<float>(m_Size.width),
            static_cast<float>(m_Size.height),
            0.0f, 1.0f,
        }, error)
        || !m_SceneRenderer.Record(*renderPass, error)) {
        return fail("Failed to record the scene draws.");
    }
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
    m_SceneRenderer.MarkSubmitted();
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
