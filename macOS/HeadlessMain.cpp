// Headless front end: renders one frame through the engine into an offscreen texture and
// saves it as a PNG artefact. Exit status: 0 = triangle drawn, 1 = failure, 2 = frame
// captured but only the clear colour is visible.

#include "Core/Types.h"
#include "Engine.h"
#include "Graphics/CapturedImage.h"
#include "HeadlessSurface.h"
#include "MacEngine.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace {

struct Pixel
{
    std::uint8_t r, g, b;
};

Pixel pixelAt(const Engine::Graphics::CapturedImage &image, std::uint32_t x, std::uint32_t y)
{
    const std::uint8_t *p = image.pixels.data()
        + (static_cast<std::size_t>(y) * image.width + x) * Engine::Graphics::BytesPerPixel(image.format);
    if (image.format == Engine::Graphics::TextureFormat::BGRA8Unorm) {
        return {p[2], p[1], p[0]};
    }
    return {p[0], p[1], p[2]};
}

// Writes the frame as an RGBA PNG and verifies the file decodes back to the same size.
bool savePng(const Engine::Graphics::CapturedImage &image, const std::filesystem::path &output)
{
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(image.width) * image.height * 4);
    for (std::uint32_t y = 0; y < image.height; ++y) {
        for (std::uint32_t x = 0; x < image.width; ++x) {
            const Pixel pixel = pixelAt(image, x, y);
            std::uint8_t *out = rgba.data() + (static_cast<std::size_t>(y) * image.width + x) * 4;
            out[0] = pixel.r;
            out[1] = pixel.g;
            out[2] = pixel.b;
            out[3] = 255;
        }
    }
    if (output.has_parent_path()) {
        std::filesystem::create_directories(output.parent_path());
    }
    const std::string path = output.string();
    if (!stbi_write_png(path.c_str(), static_cast<int>(image.width), static_cast<int>(image.height),
            4, rgba.data(), static_cast<int>(image.width * 4))) {
        return false;
    }
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc *decoded = stbi_load(path.c_str(), &width, &height, &channels, 4);
    const bool valid = decoded != nullptr && width == static_cast<int>(image.width)
        && height == static_cast<int>(image.height)
        && std::equal(rgba.begin(), rgba.end(), decoded);
    stbi_image_free(decoded);
    return valid;
}

bool sameColor(Pixel a, Pixel b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

} // namespace

int main(int argc, char **argv)
{
    std::filesystem::path output = "build/artifacts/headless.png";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--output") {
            output = argv[i + 1];
        }
    }

    std::unique_ptr<Engine::Engine> engine = Engine::Metal::CreateEngine();
    Engine::HeadlessSurface surface({512, 512});
    Engine::EngineConfig config;
    config.swapchainMode = Engine::SwapchainMode::Headless;
    Engine::Error error;
    if (engine == nullptr
        || !engine->Initialize(
            surface,
            config,
            [](const Engine::Error &runtimeError) {
                std::cerr << "Engine runtime failure: " << runtimeError.message << '\n';
            },
            error)) {
        std::cerr << "Headless initialization failed: " << error.message << '\n';
        return EXIT_FAILURE;
    }
    if (!surface.RenderFrame(error)) {
        std::cerr << "Headless render failed: " << error.message << '\n';
        return EXIT_FAILURE;
    }

    Engine::Graphics::CapturedImage image;
    if (!engine->CaptureImage(image)) {
        std::cerr << "No frame was captured.\n";
        return EXIT_FAILURE;
    }
    engine->Shutdown();

    if (!savePng(image, output)) {
        std::cerr << "Failed to write a valid PNG to " << output << '\n';
        return EXIT_FAILURE;
    }

    const Pixel background = pixelAt(image, 0, 0);
    std::size_t different = 0;
    for (std::uint32_t y = 0; y < image.height; ++y) {
        for (std::uint32_t x = 0; x < image.width; ++x) {
            if (!sameColor(pixelAt(image, x, y), background)) {
                ++different;
            }
        }
    }
    const Pixel center = pixelAt(image, image.width / 2, image.height / 2);
    std::cout << "Wrote " << output << " (" << image.width << "x" << image.height << ")\n"
              << "corner rgb=" << int(background.r) << "," << int(background.g) << ","
              << int(background.b) << " center rgb=" << int(center.r) << "," << int(center.g)
              << "," << int(center.b) << " non-background pixels=" << different << '\n';
    return different == 0 ? 2 : EXIT_SUCCESS;
}
