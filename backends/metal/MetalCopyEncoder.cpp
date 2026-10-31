#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalCopyEncoder::MetalCopyEncoder(std::shared_ptr<MetalCommandContext> context,
                                   MTL::BlitCommandEncoder *encoder)
    : context_(std::move(context)), encoder_(NS::RetainPtr(encoder))
{
}

MetalCopyEncoder::~MetalCopyEncoder()
{
    Error ignored;
    End(ignored);
}

bool MetalCopyEncoder::End(Error &error)
{
    if (ended_) {
        SetError(error, "The Metal copy encoder has already ended.");
        return false;
    }
    encoder_->endEncoding();
    ended_ = true;
    context_->EndEncoder();
    return true;
}

bool MetalCopyEncoder::CopyBuffer(const std::shared_ptr<Buffer> &source, std::size_t sourceOffset,
                                  const std::shared_ptr<Buffer> &destination,
                                  std::size_t destinationOffset, std::size_t size, Error &error)
{
    auto sourceBuffer = std::dynamic_pointer_cast<MetalBuffer>(source);
    auto destinationBuffer = std::dynamic_pointer_cast<MetalBuffer>(destination);
    if (ended_ || sourceBuffer == nullptr || destinationBuffer == nullptr || size == 0 ||
        sourceOffset > sourceBuffer->Description().size ||
        size > sourceBuffer->Description().size - sourceOffset ||
        destinationOffset > destinationBuffer->Description().size ||
        size > destinationBuffer->Description().size - destinationOffset || sourceOffset % 4 != 0 ||
        destinationOffset % 4 != 0 || size % 4 != 0 ||
        !HasBits(static_cast<std::uint32_t>(sourceBuffer->Description().usage),
                 static_cast<std::uint32_t>(BufferUsage::CopySource)) ||
        !HasBits(static_cast<std::uint32_t>(destinationBuffer->Description().usage),
                 static_cast<std::uint32_t>(BufferUsage::CopyDestination))) {
        SetError(error, "The Metal buffer copy range or usage is invalid.");
        return false;
    }
    encoder_->copyFromBuffer(sourceBuffer->Native(), sourceOffset, destinationBuffer->Native(),
                             destinationOffset, size);
    context_->Keep(source);
    context_->Keep(destination);
    return true;
}

bool MetalCopyEncoder::CopyTextureToBuffer(const std::shared_ptr<Texture> &source,
                                           const std::shared_ptr<Buffer> &destination,
                                           std::size_t destinationOffset,
                                           std::uint32_t bytesPerRow, Extent3D extent, Error &error)
{
    auto sourceTexture = std::dynamic_pointer_cast<MetalTexture>(source);
    auto destinationBuffer = std::dynamic_pointer_cast<MetalBuffer>(destination);
    if (ended_ || sourceTexture == nullptr || destinationBuffer == nullptr) {
        SetError(error, "The Metal texture copy requires a Metal texture and buffer.");
        return false;
    }
    const TextureDesc &texture = sourceTexture->Description();
    const std::uint32_t pixelSize = BytesPerPixel(texture.format);
    const std::size_t rowBytes = static_cast<std::size_t>(extent.width) * pixelSize;
    const std::size_t imageBytes = static_cast<std::size_t>(bytesPerRow) * extent.height;
    if (extent.width == 0 || extent.height == 0 || extent.depth != 1 ||
        extent.width > texture.extent.width || extent.height > texture.extent.height ||
        bytesPerRow < rowBytes || destinationOffset > destinationBuffer->Description().size ||
        imageBytes > destinationBuffer->Description().size - destinationOffset ||
        !HasBits(static_cast<std::uint32_t>(texture.usage),
                 static_cast<std::uint32_t>(TextureUsage::CopySource)) ||
        !HasBits(static_cast<std::uint32_t>(destinationBuffer->Description().usage),
                 static_cast<std::uint32_t>(BufferUsage::CopyDestination))) {
        SetError(error, "The Metal texture copy range or usage is invalid.");
        return false;
    }
    encoder_->copyFromTexture(sourceTexture->Native(), 0, 0, MTL::Origin::Make(0, 0, 0),
                              MTL::Size::Make(extent.width, extent.height, 1),
                              destinationBuffer->Native(), destinationOffset, bytesPerRow,
                              imageBytes);
    context_->Keep(source);
    context_->Keep(destination);
    return true;
}

} // namespace Metal
} // namespace Engine
