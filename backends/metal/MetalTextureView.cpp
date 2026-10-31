#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalTextureView::MetalTextureView(std::shared_ptr<Texture> texture, ResourceRange range,
                                   TextureFormat format, MTL::Texture *nativeView)
    : texture_(std::move(texture)), range_(range), format_(format),
      nativeView_(NS::RetainPtr(nativeView))
{
}

std::shared_ptr<Texture> MetalTextureView::GetTexture() const
{
    return texture_;
}

const ResourceRange &MetalTextureView::Range() const noexcept
{
    return range_;
}

TextureFormat MetalTextureView::Format() const noexcept
{
    return format_;
}

MTL::Texture *MetalTextureView::Native() const noexcept
{
    return nativeView_.get();
}

std::shared_ptr<MetalTexture> MetalTextureView::MetalTextureResource() const
{
    return std::dynamic_pointer_cast<MetalTexture>(texture_);
}

void MetalTextureView::SetDrawable(CA::MetalDrawable *drawable)
{
    drawable_ = NS::RetainPtr(drawable);
}

CA::MetalDrawable *MetalTextureView::Drawable() const noexcept
{
    return drawable_.get();
}

} // namespace Metal
} // namespace Engine
