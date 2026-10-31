#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalTexture::MetalTexture(TextureDesc description, MTL::Texture *texture)
    : description_(description), texture_(NS::RetainPtr(texture))
{
}

const TextureDesc &MetalTexture::Description() const noexcept
{
    return description_;
}

MTL::Texture *MetalTexture::Native() const noexcept
{
    return texture_.get();
}

} // namespace Metal
} // namespace Engine
