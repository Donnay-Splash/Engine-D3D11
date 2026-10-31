#include "PngEncoder.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace Engine {
namespace Graphics {
namespace {

void appendBigEndian(std::vector<std::uint8_t> &out, std::uint32_t value)
{
    out.push_back(static_cast<std::uint8_t>(value >> 24));
    out.push_back(static_cast<std::uint8_t>(value >> 16));
    out.push_back(static_cast<std::uint8_t>(value >> 8));
    out.push_back(static_cast<std::uint8_t>(value));
}

std::uint32_t crc32(const std::uint8_t *data, std::size_t size, std::uint32_t crc = 0)
{
    static const std::array<std::uint32_t, 256> s_table = [] {
        std::array<std::uint32_t, 256> table{};
        for (std::uint32_t n = 0; n < 256; ++n) {
            std::uint32_t c = n;
            for (int k = 0; k < 8; ++k) {
                c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            }
            table[n] = c;
        }
        return table;
    }();
    crc = ~crc;
    for (std::size_t i = 0; i < size; ++i) {
        crc = s_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

void appendChunk(
    std::vector<std::uint8_t> &out,
    const char (&type)[5],
    const std::vector<std::uint8_t> &payload)
{
    appendBigEndian(out, static_cast<std::uint32_t>(payload.size()));
    const std::size_t typeStart = out.size();
    out.insert(out.end(), type, type + 4);
    out.insert(out.end(), payload.begin(), payload.end());
    appendBigEndian(out, crc32(out.data() + typeStart, out.size() - typeStart));
}

} // namespace

std::vector<std::uint8_t> EncodePng(const CapturedImage &image)
{
    const std::uint32_t bytesPerPixel = BytesPerPixel(image.format);
    const bool supported = image.format == TextureFormat::RGBA8Unorm
        || image.format == TextureFormat::BGRA8Unorm || image.format == TextureFormat::R8Unorm;
    if (!supported || image.width == 0 || image.height == 0
        || image.pixels.size() < static_cast<std::size_t>(image.width) * image.height * bytesPerPixel) {
        return {};
    }

    // Filter byte 0 (None) followed by RGB rows; alpha is dropped to keep artefacts viewable.
    const std::size_t rowBytes = static_cast<std::size_t>(image.width) * 3;
    std::vector<std::uint8_t> raw;
    raw.reserve((rowBytes + 1) * image.height);
    for (std::uint32_t y = 0; y < image.height; ++y) {
        raw.push_back(0);
        const std::uint8_t *row = image.pixels.data()
            + static_cast<std::size_t>(y) * image.width * bytesPerPixel;
        for (std::uint32_t x = 0; x < image.width; ++x) {
            const std::uint8_t *pixel = row + static_cast<std::size_t>(x) * bytesPerPixel;
            if (image.format == TextureFormat::R8Unorm) {
                raw.insert(raw.end(), {pixel[0], pixel[0], pixel[0]});
            } else if (image.format == TextureFormat::BGRA8Unorm) {
                raw.insert(raw.end(), {pixel[2], pixel[1], pixel[0]});
            } else {
                raw.insert(raw.end(), {pixel[0], pixel[1], pixel[2]});
            }
        }
    }

    std::vector<std::uint8_t> zlib = {0x78, 0x01};
    constexpr std::size_t maxStored = 65535;
    std::uint32_t a = 1;
    std::uint32_t b = 0;
    for (std::uint8_t byte : raw) {
        a = (a + byte) % 65521;
        b = (b + a) % 65521;
    }
    for (std::size_t offset = 0; offset < raw.size(); offset += maxStored) {
        const std::size_t length = std::min(maxStored, raw.size() - offset);
        const bool last = offset + length == raw.size();
        zlib.push_back(last ? 1 : 0);
        zlib.push_back(static_cast<std::uint8_t>(length & 0xFF));
        zlib.push_back(static_cast<std::uint8_t>(length >> 8));
        zlib.push_back(static_cast<std::uint8_t>(~length & 0xFF));
        zlib.push_back(static_cast<std::uint8_t>((~length >> 8) & 0xFF));
        zlib.insert(zlib.end(), raw.begin() + offset, raw.begin() + offset + length);
    }
    appendBigEndian(zlib, (b << 16) | a);

    std::vector<std::uint8_t> header;
    appendBigEndian(header, image.width);
    appendBigEndian(header, image.height);
    header.insert(header.end(), {8, 2, 0, 0, 0}); // 8-bit RGB, no interlace

    std::vector<std::uint8_t> png = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    appendChunk(png, "IHDR", header);
    appendChunk(png, "IDAT", zlib);
    appendChunk(png, "IEND", {});
    return png;
}

} // namespace Graphics
} // namespace Engine
