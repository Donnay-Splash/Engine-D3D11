#include "Graphics/Adapter.h"
#include "Graphics/BindingLayout.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"
#include "Graphics/BufferView.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandContext.h"
#include "Graphics/CommandEncoder.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/ComputeEncoder.h"
#include "Graphics/ComputePipeline.h"
#include "Graphics/CopyEncoder.h"
#include "Graphics/Device.h"
#include "Graphics/DeviceCapabilities.h"
#include "Graphics/Fence.h"
#include "Graphics/GraphicsInstance.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/MemoryAllocation.h"
#include "Graphics/MemoryAllocator.h"
#include "Graphics/Pipeline.h"
#include "Graphics/PresentationSurface.h"
#include "Graphics/RenderPassEncoder.h"
#include "Graphics/Resource.h"
#include "Graphics/ResourceBarrier.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Submission.h"
#include "Graphics/Swapchain.h"
#include "Graphics/SwapchainImage.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureView.h"

#include <cassert>
#include <memory>
#include <type_traits>

namespace {

class TestResource final : public Engine::Graphics::Resource
{
};

void resourcesSupportSharedCpuOwnership()
{
    std::weak_ptr<TestResource> weak;
    {
        std::shared_ptr<TestResource> first = std::make_shared<TestResource>();
        weak = first;
        std::shared_ptr<TestResource> second = first;
        first.reset();
        assert(!weak.expired());
        second.reset();
    }
    assert(weak.expired());
}

static_assert(std::is_abstract<Engine::Graphics::Device>::value);
static_assert(std::is_abstract<Engine::Graphics::Buffer>::value);
static_assert(std::is_abstract<Engine::Graphics::Texture>::value);
static_assert(std::is_abstract<Engine::Graphics::CommandQueue>::value);
static_assert(std::is_abstract<Engine::Graphics::PresentationSurface>::value);
static_assert(
    (Engine::Graphics::BufferUsage::Vertex | Engine::Graphics::BufferUsage::Index)
    == static_cast<Engine::Graphics::BufferUsage>(3));

} // namespace

int main()
{
    resourcesSupportSharedCpuOwnership();
}
