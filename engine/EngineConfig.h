#pragma once

#include "Graphics/Swapchain.h"

namespace Engine {

struct EngineConfig
{
    Graphics::SwapchainMode swapchainMode = Graphics::SwapchainMode::Window;
};

} // namespace Engine
