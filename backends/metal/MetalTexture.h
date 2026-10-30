#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalTexture final : public Texture
{
public:
    MetalTexture(TextureDesc description, MTL::Texture *texture);

    const TextureDesc &Description() const noexcept override;

    MTL::Texture *Native() const noexcept;

private:
    TextureDesc description_;
    NS::SharedPtr<MTL::Texture> texture_;
};

} // namespace Metal
} // namespace Engine
