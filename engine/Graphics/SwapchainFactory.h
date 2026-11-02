#pragma once

#include "Swapchain.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Device;
class PresentationSurface;

// Creates the swapchain for the requested mode: the device's window swapchain for
// SwapchainMode::Window, or an OffscreenSwapchain for SwapchainMode::Headless.
std::shared_ptr<Swapchain> CreateSwapchain(
    const std::shared_ptr<Device> &device,
    PresentationSurface &surface,
    const SwapchainDesc &description,
    SwapchainMode mode,
    ::Engine::Error &error);

} // namespace Graphics
} // namespace Engine
