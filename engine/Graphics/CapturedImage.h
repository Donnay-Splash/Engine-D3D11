#pragma once

#include "GraphicsTypes.h"

#include <cstdint>
#include <vector>

namespace Engine {
namespace Graphics {

// CPU copy of a rendered frame; rows are tightly packed in the given format.
struct CapturedImage
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    TextureFormat format = TextureFormat::Unknown;
    std::vector<std::uint8_t> pixels;
};

} // namespace Graphics
} // namespace Engine
