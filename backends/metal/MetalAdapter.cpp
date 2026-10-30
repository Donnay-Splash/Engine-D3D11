#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalAdapter::MetalAdapter(std::shared_ptr<NativeDevice> native) : native_(std::move(native)) {}

const DeviceCapabilities &MetalAdapter::Capabilities() const noexcept
{
    return native_->capabilities;
}

MTL::Device *MetalAdapter::Native() const noexcept
{
    return native_->device.get();
}

std::shared_ptr<Device> MetalAdapter::CreateDevice(Error &error)
{
    AutoreleasePool pool;
    std::shared_ptr<MetalDevice> device =
        std::make_shared<MetalDevice>(NS::RetainPtr(native_->device.get()));
    if (device->GetQueue(QueueType::Graphics) == nullptr) {
        SetError(error, "Metal failed to create a graphics command queue.");
        return nullptr;
    }
    return device;
}

} // namespace Metal
} // namespace Engine
