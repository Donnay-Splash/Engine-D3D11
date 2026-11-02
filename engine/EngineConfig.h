#pragma once

namespace Engine {

enum class SwapchainMode
{
    // Present to the window surface supplied to Engine::Initialize.
    Window,
    // Render into an offscreen texture that can be read back with Engine::CaptureImage.
    Headless,
};

struct EngineConfig
{
    SwapchainMode swapchainMode = SwapchainMode::Window;
};

} // namespace Engine
