#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalCopyEncoder final : public CopyEncoder
{
public:
    MetalCopyEncoder(std::shared_ptr<MetalCommandContext> context,
                     MTL::BlitCommandEncoder *encoder);

    ~MetalCopyEncoder() override;

    bool End(Error &error) override;

    bool CopyBuffer(const std::shared_ptr<Buffer> &source, std::size_t sourceOffset,
                    const std::shared_ptr<Buffer> &destination, std::size_t destinationOffset,
                    std::size_t size, Error &error) override;

private:
    std::shared_ptr<MetalCommandContext> context_;
    NS::SharedPtr<MTL::BlitCommandEncoder> encoder_;
    bool ended_ = false;
};

} // namespace Metal
} // namespace Engine
