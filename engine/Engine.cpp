#include "Engine.h"
#include "EngineBackend.h"

#include <mutex>
#include <utility>

namespace engine {
namespace {

void appendFace(
    core::MeshData &mesh,
    core::Vector3 normal,
    core::Vector3 first,
    core::Vector3 second,
    core::Vector3 third,
    core::Vector3 fourth)
{
    const std::uint32_t firstIndex = static_cast<std::uint32_t>(mesh.positions.size());
    mesh.positions.push_back(first);
    mesh.positions.push_back(second);
    mesh.positions.push_back(third);
    mesh.positions.push_back(fourth);
    for (unsigned int vertex = 0; vertex < 4; ++vertex) {
        mesh.normals.push_back(normal);
    }
    mesh.indices.push_back(firstIndex);
    mesh.indices.push_back(firstIndex + 1);
    mesh.indices.push_back(firstIndex + 2);
    mesh.indices.push_back(firstIndex);
    mesh.indices.push_back(firstIndex + 2);
    mesh.indices.push_back(firstIndex + 3);
}

core::SceneData createInitialScene()
{
    core::SceneData scene;
    core::MeshData cube;
    appendFace(cube, {0.0f, 0.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f}, {1.0f, -1.0f, -1.0f});
    appendFace(cube, {0.0f, 0.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f});
    appendFace(cube, {0.0f, -1.0f, 0.0f},
        {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f});
    appendFace(cube, {0.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f});
    appendFace(cube, {-1.0f, 0.0f, 0.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f});
    appendFace(cube, {1.0f, 0.0f, 0.0f},
        {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},
        {1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f});
    scene.meshes.push_back(cube);

    core::MaterialData material;
    material.diffuseColor = core::Vector4(0.88f, 0.36f, 0.12f, 1.0f);
    scene.materials.push_back(material);

    core::SceneNodeData object;
    object.meshIndex = 0;
    object.materialIndex = 0;
    object.localTransform =
        core::Matrix4::rotation(core::Quaternion::axisAngle({0.0f, 1.0f, 0.0f}, 0.62f))
        * core::Matrix4::rotation(core::Quaternion::axisAngle({1.0f, 0.0f, 0.0f}, 0.38f))
        * core::Matrix4::translation({0.0f, 0.0f, 4.0f});
    scene.nodes.push_back(object);
    return scene;
}

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

Engine::Engine(std::unique_ptr<detail::Renderer> renderer)
    : renderer_(std::move(renderer)),
      scene_(createInitialScene()),
      camera_(),
      surface_(nullptr),
      errorRelay_(),
      initialized_(false),
      rendererStarted_(false),
      surfaceStarted_(false)
{
}

Engine::~Engine()
{
    shutdown();
}

bool Engine::initialize(Surface &surface, ErrorHandler errorHandler, Error &error)
{
    if (initialized_ || renderer_ == nullptr) {
        error.message = renderer_ == nullptr
            ? "The engine has no renderer implementation."
            : "The engine is already initialized.";
        return false;
    }

    surface_ = &surface;
    errorRelay_ = std::shared_ptr<ErrorRelay>(new ErrorRelay(errorHandler));
    const std::weak_ptr<ErrorRelay> weakErrorRelay(errorRelay_);
    const ErrorHandler rendererErrorHandler = [weakErrorRelay](const Error &runtimeError) {
        const std::shared_ptr<ErrorRelay> relay = weakErrorRelay.lock();
        if (relay != nullptr) {
            relay->notify(runtimeError);
        }
    };
    rendererStarted_ = true;
    if (!renderer_->initialize(
            surface.nativeHandle(),
            surface.drawableSize(),
            scene_,
            camera_,
            rendererErrorHandler,
            error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to initialize.";
        }
        errorRelay_->deactivate();
        errorRelay_.reset();
        renderer_->shutdown();
        rendererStarted_ = false;
        surface_ = nullptr;
        return false;
    }

    initialized_ = true;
    surfaceStarted_ = true;
    const Surface::Callbacks callbacks = {
        this,
        &Engine::draw,
        &Engine::resize,
        &Engine::surfaceFailure,
    };
    if (!surface.start(callbacks, error)) {
        if (error.message.empty()) {
            error.message = "The render surface failed to start.";
        }
        errorRelay_->deactivate();
        initialized_ = false;
        surface.stop();
        surfaceStarted_ = false;
        renderer_->shutdown();
        rendererStarted_ = false;
        surface_ = nullptr;
        errorRelay_.reset();
        return false;
    }
    return true;
}

void Engine::shutdown() noexcept
{
    if (errorRelay_ != nullptr) {
        errorRelay_->deactivate();
    }
    if (surfaceStarted_ && surface_ != nullptr) {
        surface_->stop();
        surfaceStarted_ = false;
    }
    initialized_ = false;
    if (rendererStarted_ && renderer_ != nullptr) {
        renderer_->shutdown();
        rendererStarted_ = false;
    }
    surface_ = nullptr;
    errorRelay_.reset();
}

bool Engine::draw(void *context, void *frame, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->initialized_
        || engine->errorRelay_ == nullptr || engine->errorRelay_->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    if (!engine->renderer_->render(frame, error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to draw a frame.";
        }
        engine->reportFailure(error);
        return false;
    }
    return true;
}

bool Engine::resize(void *context, Size size, Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine == nullptr || !engine->initialized_
        || engine->errorRelay_ == nullptr || engine->errorRelay_->hasFailed()) {
        error.message = "The engine is not running.";
        return false;
    }
    if (!engine->renderer_->resize(size, error)) {
        if (error.message.empty()) {
            error.message = "The renderer failed to resize.";
        }
        engine->reportFailure(error);
        return false;
    }
    return true;
}

void Engine::surfaceFailure(void *context, const Error &error)
{
    Engine *engine = static_cast<Engine *>(context);
    if (engine != nullptr) {
        engine->reportFailure(error);
    }
}

bool Engine::reportFailure(const Error &error)
{
    return errorRelay_ != nullptr && errorRelay_->notify(error);
}

std::unique_ptr<Engine> detail::EngineFactory::create(
    std::unique_ptr<detail::Renderer> renderer)
{
    return std::unique_ptr<Engine>(new Engine(std::move(renderer)));
}

} // namespace engine
