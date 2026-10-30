#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"
#include "SwapchainImage.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Submission;

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
};

} // namespace Graphics
} // namespace Engine
