#pragma once

#include "CapturedImage.h"
#include "Swapchain.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Buffer;
class CommandQueue;
class Device;
class Texture;

// A swapchain that renders into a texture instead of a window. "Presenting" copies the
// finished frame into CPU memory so it can be inspected or saved as an artefact.
// Built purely on the Device interfaces, so it works with any backend.
class OffscreenSwapchain final : public Swapchain
{
public:
    static std::shared_ptr<OffscreenSwapchain> Create(
        const std::shared_ptr<Device> &device,
        const SwapchainDesc &description,
        ::Engine::Error &error);

    const SwapchainDesc &Description() const noexcept override;
    std::shared_ptr<SwapchainImage> AcquireNextImage(::Engine::Error &error) override;
    bool Present(
        const std::shared_ptr<SwapchainImage> &image,
        const std::shared_ptr<Submission> &submission,
        ::Engine::Error &error) override;
    bool Resize(Extent3D extent, ::Engine::Error &error) override;

    // Returns false when no frame has been presented yet.
    bool LatestImage(CapturedImage &image) const;

private:
    OffscreenSwapchain(std::shared_ptr<Device> device, SwapchainDesc description);
    bool CreateTargets(::Engine::Error &error);

    std::shared_ptr<Device> m_Device;
    std::shared_ptr<CommandQueue> m_Queue;
    SwapchainDesc m_Description;
    std::shared_ptr<SwapchainImage> m_Image;
    std::shared_ptr<Texture> m_Texture;
    std::shared_ptr<Buffer> m_ReadbackBuffer;
    std::uint32_t m_BytesPerRow;
    CapturedImage m_Latest;
    bool m_HasLatest;
};

} // namespace Graphics
} // namespace Engine
