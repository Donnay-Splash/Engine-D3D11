#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalAdapter final : public Adapter
{
public:
    explicit MetalAdapter(std::shared_ptr<NativeDevice> native);

    const DeviceCapabilities &Capabilities() const noexcept override;

    std::shared_ptr<Device> CreateDevice(Error &error) override;
    MTL::Device *Native() const noexcept;

private:
    std::shared_ptr<NativeDevice> native_;
};

} // namespace Metal
} // namespace Engine
