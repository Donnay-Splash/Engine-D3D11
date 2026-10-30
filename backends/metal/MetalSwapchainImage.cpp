#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalSwapchainImage::MetalSwapchainImage(NS::SharedPtr<CA::MetalDrawable> drawable,
                                         std::shared_ptr<MetalTextureView> view)
    : drawable_(std::move(drawable)), view_(std::move(view))
{
}

std::shared_ptr<TextureView> MetalSwapchainImage::View() const
{
    return view_;
}

CA::MetalDrawable *MetalSwapchainImage::Drawable() const noexcept
{
    return drawable_.get();
}

std::shared_ptr<MetalTextureView> MetalSwapchainImage::MetalView() const noexcept
{
    return view_;
}

} // namespace Metal
} // namespace Engine
