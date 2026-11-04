#include "Rendering/ImageLoader.h"

#include "Rendering/TextureUpload.h"

#include <fstream>
#include <iterator>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Engine {

bool ImageLoader::Decode(const std::uint8_t *data, std::size_t size, Image &image, Error &error)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc *decoded = data == nullptr
        ? nullptr
        : stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 4);
    if (decoded == nullptr) {
        const char *reason = stbi_failure_reason();
        error.message = std::string("Failed to decode the image: ") + (reason ? reason : "no data");
        return false;
    }
    image.width = static_cast<std::uint32_t>(width);
    image.height = static_cast<std::uint32_t>(height);
    image.pixels.assign(decoded, decoded + static_cast<std::size_t>(width) * height * 4);
    stbi_image_free(decoded);
    return true;
}

bool ImageLoader::DecodeFile(const std::string &path, Image &image, Error &error)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        error.message = "Failed to open the image '" + path + "'.";
        return false;
    }
    const std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (!Decode(bytes.data(), bytes.size(), image, error)) {
        error.message += " (" + path + ")";
        return false;
    }
    return true;
}

std::shared_ptr<Graphics::TextureView> ImageLoader::LoadTexture(
    Graphics::Device &device, const std::string &path, Error &error)
{
    Image image;
    if (!DecodeFile(path, image, error)) {
        return nullptr;
    }
    return CreateTexture2D(
        device, Graphics::TextureFormat::RGBA8Unorm, image.width, image.height,
        image.pixels.data(), error);
}

} // namespace Engine
