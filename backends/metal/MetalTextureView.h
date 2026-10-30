#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalTextureView final : public TextureView
{
public:
    MetalTextureView(std::shared_ptr<Texture> texture, ResourceRange range, TextureFormat format,
                     MTL::Texture *nativeView);

    std::shared_ptr<Texture> GetTexture() const override;

    const ResourceRange &Range() const noexcept override;

    TextureFormat Format() const noexcept override;

    MTL::Texture *Native() const noexcept;

    std::shared_ptr<MetalTexture> MetalTextureResource() const;

    void SetDrawable(CA::MetalDrawable *drawable);

    CA::MetalDrawable *Drawable() const noexcept;

private:
    std::shared_ptr<Texture> texture_;
    ResourceRange range_;
    TextureFormat format_;
    NS::SharedPtr<MTL::Texture> nativeView_;
    NS::SharedPtr<CA::MetalDrawable> drawable_;
};

} // namespace Metal
} // namespace Engine
