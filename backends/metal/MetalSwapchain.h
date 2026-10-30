#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalSwapchain final : public Swapchain
{
public:
    MetalSwapchain(CA::MetalLayer *layer, MTL::Device *device, SwapchainDesc description);

    const SwapchainDesc &Description() const noexcept override;

    std::shared_ptr<SwapchainImage> AcquireNextImage(Error &error) override;

    bool Present(const std::shared_ptr<SwapchainImage> &image,
                 const std::shared_ptr<Submission> &submission, Error &error) override;

    bool Resize(Extent3D extent, Error &error) override;

private:
    void ConfigureLayer();

    NS::SharedPtr<CA::MetalLayer> layer_;
    MTL::Device *device_;
    SwapchainDesc description_;
};

} // namespace Metal
} // namespace Engine
