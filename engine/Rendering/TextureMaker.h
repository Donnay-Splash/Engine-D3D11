#pragma once

#include "Core/Types.h"

#include <cstdint>
#include <memory>

namespace Engine {
namespace Graphics {
class Device;
class TextureView;
} // namespace Graphics

// Procedural textures for tests and placeholders.
class TextureMaker
{
public:
    // Alternating black-and-white squares of `cellSize` pixels.
    static std::shared_ptr<Graphics::TextureView> CreateCheckerboard(
        Graphics::Device &device,
        std::uint32_t size,
        std::uint32_t cellSize,
        Error &error);
};

} // namespace Engine
