#pragma once
#include "Engine.h"
#include "Device.h"

#include <memory>

namespace Engine {

std::unique_ptr<Device> CreateMetalDevice();

} // namespace Engine
