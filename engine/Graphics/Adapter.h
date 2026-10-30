#pragma once

#include "DeviceCapabilities.h"
#include "Resource.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Device;

class Adapter : public Resource
{
public:
    virtual const DeviceCapabilities &Capabilities() const noexcept = 0;
    virtual std::shared_ptr<Device> CreateDevice(::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
