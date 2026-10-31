#pragma once

#include "CapturedImage.h"

#include <cstdint>
#include <vector>

namespace Engine {
namespace Graphics {

// Encodes an 8-bit RGBA/BGRA/R image as an uncompressed (stored-deflate) PNG.
// Returns an empty vector for unsupported formats or empty images.
std::vector<std::uint8_t> EncodePng(const CapturedImage &image);

} // namespace Graphics
} // namespace Engine
