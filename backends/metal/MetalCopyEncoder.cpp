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

} // namespace Metal
} // namespace Engine
