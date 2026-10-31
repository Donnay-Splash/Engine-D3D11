#include "OffscreenSwapchain.h"

#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandContext.h"
#include "CommandQueue.h"
#include "Core/Types.h"
#include "CopyEncoder.h"
#include "Device.h"
#include "Submission.h"
#include "Texture.h"
#include "TextureView.h"

#include <cstring>
#include <utility>

namespace Engine {
namespace Graphics {
namespace {

class OffscreenImage final : public SwapchainImage
{
public:
    explicit OffscreenImage(std::shared_ptr<TextureView> view) : m_View(std::move(view)) {}

    std::shared_ptr<TextureView> View() const override { return m_View; }

private:
    std::shared_ptr<TextureView> m_View;
};

} // namespace

OffscreenSwapchain::OffscreenSwapchain(std::shared_ptr<Device> device, SwapchainDesc description)
    : m_Device(std::move(device)),
      m_Description(description),
      m_BytesPerRow(0),
      m_HasLatest(false)
{
}

std::shared_ptr<OffscreenSwapchain> OffscreenSwapchain::Create(
    const std::shared_ptr<Device> &device,
    const SwapchainDesc &description,
    ::Engine::Error &error)
{
    if (device == nullptr || description.extent.width == 0 || description.extent.height == 0
        || description.extent.depth != 1 || BytesPerPixel(description.format) == 0
        || description.format == TextureFormat::Depth32Float) {
        error.message = "The offscreen swapchain description is invalid.";
        return nullptr;
    }
    std::shared_ptr<OffscreenSwapchain> swapchain(
        new OffscreenSwapchain(device, description));
    swapchain->m_Queue = device->GetQueue(QueueType::Graphics);
    if (swapchain->m_Queue == nullptr) {
        error.message = "The offscreen swapchain requires a graphics queue.";
        return nullptr;
    }
    if (!swapchain->CreateTargets(error)) {
        return nullptr;
    }
    return swapchain;
}

bool OffscreenSwapchain::CreateTargets(::Engine::Error &error)
{
    TextureDesc textureDescription;
    textureDescription.format = m_Description.format;
    textureDescription.extent = m_Description.extent;
    textureDescription.usage = TextureUsage::ColorAttachment | TextureUsage::CopySource;
    textureDescription.memoryType = MemoryType::DeviceLocal;
    std::shared_ptr<Texture> texture = m_Device->CreateTexture(textureDescription, error);
    if (texture == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the offscreen color texture.";
        }
        return false;
    }
    std::shared_ptr<TextureView> view =
        m_Device->CreateTextureView(texture, ResourceRange(), m_Description.format, error);
    if (view == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the offscreen color texture view.";
        }
        return false;
    }

    const std::uint32_t bytesPerRow =
        m_Description.extent.width * BytesPerPixel(m_Description.format);
    BufferDesc readbackDescription;
    readbackDescription.size = static_cast<std::size_t>(bytesPerRow) * m_Description.extent.height;
    readbackDescription.usage = BufferUsage::CopyDestination;
    readbackDescription.memoryType = MemoryType::Readback;
    std::shared_ptr<Buffer> readback = m_Device->CreateBuffer(readbackDescription, error);
    if (readback == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create the offscreen readback buffer.";
        }
        return false;
    }

    m_Texture = std::move(texture);
    m_ReadbackBuffer = std::move(readback);
    m_BytesPerRow = bytesPerRow;
    m_Image = std::make_shared<OffscreenImage>(std::move(view));
    return true;
}

const SwapchainDesc &OffscreenSwapchain::Description() const noexcept
{
    return m_Description;
}

std::shared_ptr<SwapchainImage> OffscreenSwapchain::AcquireNextImage(::Engine::Error &)
{
    // Present waits for the GPU, so a single image is never in flight when it is re-acquired.
    return m_Image;
}

bool OffscreenSwapchain::Present(
    const std::shared_ptr<SwapchainImage> &image,
    const std::shared_ptr<Submission> &submission,
    ::Engine::Error &error)
{
    if (image != m_Image || submission == nullptr) {
        error.message = "Offscreen presentation requires the acquired image and its submission.";
        return false;
    }

    std::shared_ptr<CommandContext> context = m_Queue->CreateContext(error);
    if (context == nullptr) {
        return false;
    }
    std::unique_ptr<CopyEncoder> copy = context->BeginCopy(error);
    if (copy == nullptr) {
        return false;
    }
    if (!copy->CopyTextureToBuffer(
            m_Texture, m_ReadbackBuffer, 0, m_BytesPerRow, m_Description.extent, error)
        || !copy->End(error)) {
        return false;
    }
    std::shared_ptr<CommandBuffer> commands = context->Finish(error);
    if (commands == nullptr) {
        return false;
    }
    std::shared_ptr<Submission> readback = m_Queue->Submit(commands, {submission}, {}, error);
    if (readback == nullptr || !readback->Wait(error)) {
        if (error.message.empty()) {
            error.message = "The offscreen readback copy failed.";
        }
        return false;
    }

    void *data = nullptr;
    if (!m_ReadbackBuffer->Map(data, error)) {
        return false;
    }
    if (data == nullptr) {
        m_ReadbackBuffer->Unmap();
        error.message = "The offscreen readback buffer returned no mapped memory.";
        return false;
    }
    m_Latest.width = m_Description.extent.width;
    m_Latest.height = m_Description.extent.height;
    m_Latest.format = m_Description.format;
    m_Latest.pixels.resize(m_ReadbackBuffer->Description().size);
    std::memcpy(m_Latest.pixels.data(), data, m_Latest.pixels.size());
    m_ReadbackBuffer->Unmap();
    m_HasLatest = true;
    return true;
}

bool OffscreenSwapchain::Resize(Extent3D extent, ::Engine::Error &error)
{
    if (extent.width == 0 || extent.height == 0 || extent.depth != 1) {
        error.message = "The offscreen swapchain extent must be nonzero with depth one.";
        return false;
    }
    const SwapchainDesc previous = m_Description;
    m_Description.extent = extent;
    if (!CreateTargets(error)) {
        m_Description = previous;
        return false;
    }
    m_HasLatest = false;
    return true;
}

bool OffscreenSwapchain::LatestImage(CapturedImage &image) const
{
    if (!m_HasLatest) {
        return false;
    }
    image = m_Latest;
    return true;
}

} // namespace Graphics
} // namespace Engine
