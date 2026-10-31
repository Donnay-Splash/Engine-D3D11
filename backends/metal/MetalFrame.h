#pragma once

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLRenderPass.hpp>
#include <QuartzCore/CAMetalDrawable.hpp>

namespace Engine {
namespace Metal {

struct MetalFrame
{
    NS::SharedPtr<CA::MetalDrawable> drawable;
    NS::SharedPtr<MTL::RenderPassDescriptor> renderPass;
};

} // namespace Metal
} // namespace Engine
