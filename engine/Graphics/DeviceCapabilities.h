#pragma once

#include "GraphicsTypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Engine {
namespace Graphics {

struct DeviceCapabilities
{
    std::string name;
    std::uint32_t maxTextureDimension2D = 0;
    std::uint32_t maxColorAttachments = 0;
    std::vector<QueueType> queueTypes;
    std::vector<TextureFormat> supportedTextureFormats;
    bool supportsCompute = false;
    bool supportsIndirectDrawing = false;
};

} // namespace Graphics
} // namespace Engine
