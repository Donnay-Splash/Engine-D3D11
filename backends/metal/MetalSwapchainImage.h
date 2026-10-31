#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalSwapchainImage final : public SwapchainImage
{
public:
    MetalSwapchainImage(NS::SharedPtr<CA::MetalDrawable> drawable,
                        std::shared_ptr<MetalTextureView> view);

    std::shared_ptr<TextureView> View() const override;

    CA::MetalDrawable *Drawable() const noexcept;

    std::shared_ptr<MetalTextureView> MetalView() const noexcept;

private:
    NS::SharedPtr<CA::MetalDrawable> drawable_;
    std::shared_ptr<MetalTextureView> view_;
};

} // namespace Metal
} // namespace Engine
