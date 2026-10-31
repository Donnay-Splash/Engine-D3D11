#pragma once

#include "Core/Mesh.h"
#include "Core/Types.h"
#include "Graphics/CapturedImage.h"
#include "Graphics/PresentationSurface.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace Engine {

class Engine;
class ShaderLibrary;
namespace Graphics {
class BindingSet;
class Buffer;
class BufferView;
class Device;
class GraphicsInstance;
class GraphicsPipeline;
class OffscreenSwapchain;
class Swapchain;
class CommandQueue;
class Adapter;
}

class Surface : public Graphics::PresentationSurface
{
public:
    struct Callbacks
    {
        void *context;
        bool (*Draw)(void *context, Error &error);
        bool (*Resize)(void *context, Size size, Error &error);
        void (*Failure)(void *context, const Error &error);
    };

    virtual ~Surface() = default;
    virtual Size DrawableSize() const noexcept = 0;
    Graphics::Extent3D DrawableExtent() const noexcept override
    {
        const Size size = DrawableSize();
        Graphics::Extent3D extent;
        extent.width = size.width;
        extent.height = size.height;
        extent.depth = 1;
        return extent;
    }
    virtual bool Start(const Callbacks &callbacks, Error &error) = 0;
    virtual void Stop() noexcept = 0;
};

class Engine
{
public:
    using ErrorHandler = ::Engine::ErrorHandler;

    ~Engine();

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;

    bool Initialize(Surface &surface, ErrorHandler errorHandler, Error &error);
    // Headless mode runs the same frame flow as a windowed engine, but renders into an
    // offscreen texture. Drive it with RenderHeadlessFrame and read the result back.
    bool InitializeHeadless(Size size, ErrorHandler errorHandler, Error &error);
    bool RenderHeadlessFrame(Error &error);
    bool CaptureHeadlessImage(Graphics::CapturedImage &image) const;
    void Shutdown() noexcept;

private:
    struct ErrorRelay;

    Engine(
        std::shared_ptr<Graphics::GraphicsInstance> graphicsInstance,
        std::shared_ptr<ShaderLibrary> shaderLibrary,
        std::shared_ptr<Graphics::Adapter> preferredAdapter);
    bool InitializeGraphics(Surface *surface, Size size, Error &error);
    bool DrawFrame(Error &error);
    static bool Draw(void *context, Error &error);
    static bool Resize(void *context, Size size, Error &error);
    static void SurfaceFailure(void *context, const Error &error);

    bool ReportFailure(const Error &error);

    friend class EngineFactory;

    std::shared_ptr<Graphics::GraphicsInstance> m_GraphicsInstance;
    std::shared_ptr<ShaderLibrary> m_ShaderLibrary;
    std::shared_ptr<Graphics::Adapter> m_PreferredAdapter;
    std::shared_ptr<Graphics::Device> m_Device;
    std::shared_ptr<Graphics::CommandQueue> m_GraphicsQueue;
    std::shared_ptr<Graphics::Swapchain> m_Swapchain;
    std::shared_ptr<Graphics::OffscreenSwapchain> m_OffscreenSwapchain;
    std::shared_ptr<Graphics::GraphicsPipeline> m_GraphicsPipeline;
    std::shared_ptr<Graphics::BindingSet> m_Bindings;
    std::shared_ptr<Graphics::Buffer> m_TransformBuffer;
    std::shared_ptr<Graphics::BufferView> m_TransformBufferView;
    Mesh m_Mesh;
    Surface *m_Surface;
    Size m_Size;
    std::shared_ptr<ErrorRelay> m_ErrorRelay;
    bool m_Initialized;
    bool m_SurfaceStarted;
    bool m_TransformBufferPrepared;
};

} // namespace engine
