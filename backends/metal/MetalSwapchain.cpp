#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalSwapchain::MetalSwapchain(CA::MetalLayer *layer, MTL::Device *device,
                               SwapchainDesc description)
    : layer_(NS::RetainPtr(layer)), device_(device), description_(description)
{
    ConfigureLayer();
}

const SwapchainDesc &MetalSwapchain::Description() const noexcept
{
    return description_;
}

std::shared_ptr<SwapchainImage> MetalSwapchain::AcquireNextImage(Error &error)
{
    AutoreleasePool pool;
    if (description_.extent.width == 0 || description_.extent.height == 0) {
        SetError(error, "Cannot acquire a Metal drawable for a zero-sized swapchain.");
        return nullptr;
    }
    CA::MetalDrawable *drawable = layer_->nextDrawable();
    if (drawable == nullptr) {
        SetError(error, "The Metal layer did not provide a drawable.");
        return nullptr;
    }
    MTL::Texture *nativeTexture = drawable->texture();
    if (nativeTexture == nullptr) {
        SetError(error, "The Metal drawable has no texture.");
        return nullptr;
    }
    TextureDesc textureDescription;
    textureDescription.format = description_.format;
    textureDescription.extent = description_.extent;
    textureDescription.usage = TextureUsage::ColorAttachment | TextureUsage::CopySource;
    auto texture = std::make_shared<MetalTexture>(textureDescription, nativeTexture);
    ResourceRange range;
    auto view =
        std::make_shared<MetalTextureView>(texture, range, description_.format, nativeTexture);
    view->SetDrawable(drawable);
    return std::make_shared<MetalSwapchainImage>(NS::RetainPtr(drawable), std::move(view));
}

bool MetalSwapchain::Present(const std::shared_ptr<SwapchainImage> &image,
                             const std::shared_ptr<Submission> &submission, Error &error)
{
    auto metalImage = std::dynamic_pointer_cast<MetalSwapchainImage>(image);
    auto metalSubmission = std::dynamic_pointer_cast<MetalSubmission>(submission);
    if (metalImage == nullptr || metalSubmission == nullptr ||
        metalSubmission->Commands() == nullptr) {
        SetError(error, "Metal presentation requires a Metal image and submission.");
        return false;
    }
    if (!metalSubmission->Commands()->HasPresented(metalImage->Drawable())) {
        SetError(error, "The submission did not render or schedule this Metal drawable.");
        return false;
    }
    return true;
}

bool MetalSwapchain::Resize(Extent3D extent, Error &error)
{
    if (extent.width == 0 || extent.height == 0 || extent.depth != 1) {
        SetError(error,
                 "The Metal swapchain extent must have nonzero width and height and depth one.");
        return false;
    }
    description_.extent = extent;
    ConfigureLayer();
    return true;
}

void MetalSwapchain::ConfigureLayer()
{
    layer_->setDevice(device_);
    layer_->setPixelFormat(ToMetal(description_.format));
    layer_->setFramebufferOnly(false);
    layer_->setDrawableSize(CGSizeMake(static_cast<CGFloat>(description_.extent.width),
                                       static_cast<CGFloat>(description_.extent.height)));
    layer_->setMaximumDrawableCount(description_.imageCount);
}

} // namespace Metal
} // namespace Engine
