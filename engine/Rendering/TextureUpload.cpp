#include "Rendering/TextureUpload.h"

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandContext.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/CopyEncoder.h"
#include "Graphics/Device.h"
#include "Graphics/Submission.h"
#include "Graphics/Texture.h"
#include "Graphics/TextureView.h"

#include <cstring>

namespace Engine {

namespace {

std::shared_ptr<Graphics::TextureView> fail(Error &error, const char *fallback)
{
    if (error.message.empty()) {
        error.message = fallback;
    }
    return nullptr;
}

} // namespace

std::shared_ptr<Graphics::TextureView> CreateTexture2D(
    Graphics::Device &device,
    Graphics::TextureFormat format,
    std::uint32_t width,
    std::uint32_t height,
    const void *pixels,
    Error &error)
{
    const std::uint32_t pixelSize = Graphics::BytesPerPixel(format);
    if (pixels == nullptr || width == 0 || height == 0 || pixelSize == 0) {
        error.message = "A texture upload needs pixels, a nonzero extent and a color format.";
        return nullptr;
    }
    const std::size_t rowBytes = static_cast<std::size_t>(width) * pixelSize;
    const std::size_t size = rowBytes * height;

    Graphics::BufferDesc stagingDescription;
    stagingDescription.size = size;
    stagingDescription.usage = Graphics::BufferUsage::CopySource;
    stagingDescription.memoryType = Graphics::MemoryType::Upload;
    std::shared_ptr<Graphics::Buffer> staging = device.CreateBuffer(stagingDescription, error);
    if (staging == nullptr) {
        return fail(error, "Failed to create the texture staging buffer.");
    }
    void *mapped = nullptr;
    if (!staging->Map(mapped, error)) {
        return nullptr;
    }
    if (mapped == nullptr) {
        staging->Unmap();
        error.message = "The texture staging buffer returned no mapped memory.";
        return nullptr;
    }
    std::memcpy(mapped, pixels, size);
    staging->Unmap();

    Graphics::TextureDesc description;
    description.format = format;
    description.extent = {width, height, 1};
    description.usage = Graphics::TextureUsage::Sampled | Graphics::TextureUsage::CopyDestination;
    description.memoryType = Graphics::MemoryType::DeviceLocal;
    std::shared_ptr<Graphics::Texture> texture = device.CreateTexture(description, error);
    if (texture == nullptr) {
        return fail(error, "Failed to create the texture.");
    }
    std::shared_ptr<Graphics::TextureView> view =
        device.CreateTextureView(texture, Graphics::ResourceRange{}, format, error);
    if (view == nullptr) {
        return fail(error, "Failed to create a texture view.");
    }

    std::shared_ptr<Graphics::CommandQueue> queue =
        device.GetQueue(Graphics::QueueType::Graphics);
    if (queue == nullptr) {
        error.message = "The graphics device has no graphics command queue.";
        return nullptr;
    }
    std::shared_ptr<Graphics::CommandContext> context = queue->CreateContext(error);
    if (context == nullptr) {
        return fail(error, "Failed to create a command context for texture upload.");
    }
    Graphics::ResourceBarrierDesc barrier;
    barrier.before = Graphics::ResourceUsage::Undefined;
    barrier.after = Graphics::ResourceUsage::CopySource;
    if (!context->Barrier(staging, barrier, error)) {
        return fail(error, "Failed to prepare the texture staging buffer.");
    }
    barrier.after = Graphics::ResourceUsage::CopyDestination;
    if (!context->Barrier(texture, barrier, error)) {
        return fail(error, "Failed to prepare the texture for upload.");
    }
    std::unique_ptr<Graphics::CopyEncoder> encoder = context->BeginCopy(error);
    if (encoder == nullptr) {
        return fail(error, "Failed to begin the texture upload.");
    }
    if (!encoder->CopyBufferToTexture(
            staging, 0, static_cast<std::uint32_t>(rowBytes), texture, description.extent, error)
        || !encoder->End(error)) {
        return fail(error, "Failed to record the texture upload.");
    }
    barrier.before = Graphics::ResourceUsage::CopyDestination;
    barrier.after = Graphics::ResourceUsage::ShaderRead;
    if (!context->Barrier(texture, barrier, error)) {
        return fail(error, "Failed to make the texture readable by shaders.");
    }
    std::shared_ptr<Graphics::CommandBuffer> commandBuffer = context->Finish(error);
    if (commandBuffer == nullptr) {
        return fail(error, "Failed to finish the texture upload command buffer.");
    }
    std::shared_ptr<Graphics::Submission> submission =
        queue->Submit(commandBuffer, {}, {}, error);
    if (submission == nullptr) {
        return fail(error, "Failed to submit the texture upload.");
    }
    if (!submission->Wait(error)) {
        return fail(error, "The texture upload failed.");
    }
    return view;
}

} // namespace Engine
