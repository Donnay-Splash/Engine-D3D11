#include "SwapchainFactory.h"

#include "Core/Types.h"
#include "Device.h"
#include "OffscreenSwapchain.h"

namespace Engine {
namespace Graphics {

std::shared_ptr<Swapchain> CreateSwapchain(
    const std::shared_ptr<Device> &device,
    PresentationSurface &surface,
    const SwapchainDesc &description,
    SwapchainMode mode,
    ::Engine::Error &error)
{
    if (device == nullptr) {
        error.message = "A swapchain requires a graphics device.";
        return nullptr;
    }
    switch (mode) {
    case SwapchainMode::Headless:
        return OffscreenSwapchain::Create(device, description, error);
    case SwapchainMode::Window:
        return device->CreateSwapchain(surface, description, error);
    }
    error.message = "The swapchain mode is not supported.";
    return nullptr;
}

} // namespace Graphics
} // namespace Engine
