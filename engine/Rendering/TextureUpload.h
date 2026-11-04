#pragma once

#include "Core/Types.h"
#include "Graphics/GraphicsTypes.h"

#include <cstdint>
#include <memory>

namespace Engine {
namespace Graphics {
class Device;
class TextureView;
} // namespace Graphics

// Creates a sampled 2D texture from tightly packed pixels (`BytesPerPixel(format) * width`
// per row) and blocks until the upload has finished, leaving it ready for shader reads.
// Intended for load-time use; see issue #30 for a non-blocking path.
std::shared_ptr<Graphics::TextureView> CreateTexture2D(
    Graphics::Device &device,
    Graphics::TextureFormat format,
    std::uint32_t width,
    std::uint32_t height,
    const void *pixels,
    Error &error);

} // namespace Engine
