// Headless front end: renders one frame through the engine into an offscreen texture and
// saves it as a PNG artefact. Exit status: 0 = triangle drawn, 1 = failure, 2 = frame
// captured but only the clear colour is visible.

#include "Core/Types.h"
#include "Engine.h"
#include "Graphics/CapturedImage.h"
#include "Graphics/PngEncoder.h"
#include "MacEngine.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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
    Engine::Error error;
    if (engine == nullptr
        || !engine->InitializeHeadless(
            {512, 512},
            [](const Engine::Error &runtimeError) {
                std::cerr << "Engine runtime failure: " << runtimeError.message << '\n';
            },
            error)) {
        std::cerr << "Headless initialization failed: " << error.message << '\n';
        return EXIT_FAILURE;
    }
    if (!engine->RenderHeadlessFrame(error)) {
        std::cerr << "Headless render failed: " << error.message << '\n';
        return EXIT_FAILURE;
    }

    Engine::Graphics::CapturedImage image;
    if (!engine->CaptureHeadlessImage(image)) {
        std::cerr << "No frame was captured.\n";
        return EXIT_FAILURE;
    }
    engine->Shutdown();

    const std::vector<std::uint8_t> png = Engine::Graphics::EncodePng(image);
    if (png.empty()) {
        std::cerr << "Failed to encode the captured frame.\n";
        return EXIT_FAILURE;
    }
    if (output.has_parent_path()) {
        std::filesystem::create_directories(output.parent_path());
    }
    std::ofstream file(output, std::ios::binary);
    file.write(reinterpret_cast<const char *>(png.data()), static_cast<std::streamsize>(png.size()));
    if (!file) {
        std::cerr << "Failed to write " << output << '\n';
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
