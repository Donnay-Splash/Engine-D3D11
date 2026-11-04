#pragma once

#include "Core/Types.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Engine {
namespace Graphics {
class Device;
class TextureView;
} // namespace Graphics

// A decoded image: tightly packed RGBA8 pixels, top row first.
struct Image
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> pixels;
};

class ImageLoader
{
public:
    // Decodes PNG/JPEG/BMP/TGA data (anything stb_image supports) to RGBA8.
    static bool Decode(const std::uint8_t *data, std::size_t size, Image &image, Error &error);
    static bool DecodeFile(const std::string &path, Image &image, Error &error);

    // Decodes a file and uploads it as a sampled RGBA8 texture (blocking, see CreateTexture2D).
    static std::shared_ptr<Graphics::TextureView> LoadTexture(
        Graphics::Device &device, const std::string &path, Error &error);
};

} // namespace Engine
