#pragma once

#include "CommandEncoder.h"
#include "GraphicsTypes.h"

#include <cstddef>
#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Buffer;
class Texture;

class CopyEncoder : public CommandEncoder
{
public:
    virtual bool CopyBuffer(
        const std::shared_ptr<Buffer> &source,
        std::size_t sourceOffset,
        const std::shared_ptr<Buffer> &destination,
        std::size_t destinationOffset,
        std::size_t size,
        ::Engine::Error &error) = 0;
    // Copies mip 0, layer 0 of a texture into tightly described rows of a buffer.
    virtual bool CopyTextureToBuffer(
        const std::shared_ptr<Texture> &source,
        const std::shared_ptr<Buffer> &destination,
        std::size_t destinationOffset,
        std::uint32_t bytesPerRow,
        Extent3D extent,
        ::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
