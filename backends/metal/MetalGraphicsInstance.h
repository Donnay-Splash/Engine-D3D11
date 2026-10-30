#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalGraphicsInstance final : public GraphicsInstance
{
public:
    bool EnumerateAdapters(std::vector<std::shared_ptr<Adapter>> &adapters, Error &error) override;

private:
    std::mutex mutex_;
    std::vector<std::shared_ptr<MetalAdapter>> adapterCache_;
};

} // namespace Metal
} // namespace Engine
