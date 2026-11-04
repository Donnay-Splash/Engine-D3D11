#include "Rendering/TextureMaker.h"

#include "Rendering/TextureUpload.h"

#include <vector>

namespace Engine {

std::shared_ptr<Graphics::TextureView> TextureMaker::CreateCheckerboard(
    Graphics::Device &device,
    std::uint32_t size,
    std::uint32_t cellSize,
    Error &error)
{
    if (size == 0 || cellSize == 0) {
        error.message = "A checkerboard needs a nonzero size and cell size.";
        return nullptr;
    }
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(size) * size * 4);
    for (std::uint32_t y = 0; y < size; ++y) {
        for (std::uint32_t x = 0; x < size; ++x) {
            const std::uint8_t value = ((x / cellSize) + (y / cellSize)) % 2 == 0 ? 255 : 32;
            std::uint8_t *pixel = &pixels[(static_cast<std::size_t>(y) * size + x) * 4];
            pixel[0] = pixel[1] = pixel[2] = value;
            pixel[3] = 255;
        }
    }
    return CreateTexture2D(
        device, Graphics::TextureFormat::RGBA8Unorm, size, size, pixels.data(), error);
}

} // namespace Engine
