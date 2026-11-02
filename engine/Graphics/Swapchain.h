#pragma once

#include "CapturedImage.h"
#include "GraphicsTypes.h"
#include "Resource.h"
#include "SwapchainImage.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Submission;

enum class SwapchainMode
{
    // Present to a window surface.
    Window,
    // Render into an offscreen texture that can be read back with CaptureLatestImage.
    Headless,
};

struct SwapchainDesc
{
    Extent3D extent;
    TextureFormat format = TextureFormat::BGRA8Unorm;
    std::uint32_t imageCount = 3;
};

class Swapchain : public Resource
{
public:
    virtual const SwapchainDesc &Description() const noexcept = 0;
    virtual std::shared_ptr<SwapchainImage> AcquireNextImage(::Engine::Error &error) = 0;
    virtual bool Present(
        const std::shared_ptr<SwapchainImage> &image,
        const std::shared_ptr<Submission> &submission,
        ::Engine::Error &error) = 0;
    virtual bool Resize(Extent3D extent, ::Engine::Error &error) = 0;
    // Copies the most recently presented frame to CPU memory. Returns false when no frame is
    // available or the swapchain does not support capture.
    virtual bool CaptureLatestImage(CapturedImage &) const { return false; }
};

} // namespace Graphics
} // namespace Engine
