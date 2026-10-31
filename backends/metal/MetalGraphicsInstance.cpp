#include "MetalBackendInternal.h"
#include "MetalBackend.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

bool MetalGraphicsInstance::EnumerateAdapters(std::vector<std::shared_ptr<Adapter>> &adapters,
                                              Error &error)
{
    AutoreleasePool pool;
    NS::SharedPtr<MTL::Device> defaultDevice = NS::RetainPtr(MTL::CreateSystemDefaultDevice());
    NS::SharedPtr<NS::Array> allDevices = NS::TransferPtr(MTL::CopyAllDevices());
    if (!defaultDevice && (!allDevices || allDevices->count() == 0)) {
        SetError(error, "Metal did not return any available devices.");
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Adapter>> enumerated;
    auto appendDevice = [this, &enumerated](MTL::Device *device) {
        if (device == nullptr) {
            return;
        }
        auto found = std::find_if(adapterCache_.begin(), adapterCache_.end(),
                                  [device](const std::shared_ptr<MetalAdapter> &adapter) {
                                      return adapter->Native() == device;
                                  });
        if (found == adapterCache_.end()) {
            std::shared_ptr<NativeDevice> native = std::make_shared<NativeDevice>(device);
            adapterCache_.push_back(std::make_shared<MetalAdapter>(std::move(native)));
            found = std::prev(adapterCache_.end());
        }
        if (std::find(enumerated.begin(), enumerated.end(), *found) == enumerated.end()) {
            enumerated.push_back(*found);
        }
    };

    appendDevice(defaultDevice.get());
    if (allDevices) {
        for (NS::UInteger index = 0; index < allDevices->count(); ++index) {
            appendDevice(static_cast<MTL::Device *>(allDevices->object(index)));
        }
    }
    adapters = std::move(enumerated);
    return true;
}

std::shared_ptr<GraphicsInstance> CreateMetalGraphicsInstance()
{
    static std::shared_ptr<GraphicsInstance> instance = std::make_shared<MetalGraphicsInstance>();
    return instance;
}

} // namespace Metal
} // namespace Engine
