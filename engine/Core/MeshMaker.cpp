#include "MeshMaker.h"

#include "Core/Mesh.h"
#include "Core/Types.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandContext.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/CopyEncoder.h"
#include "Graphics/Device.h"

#include <cstring>
#include <memory>
#include <vector>

namespace Engine {
namespace {

struct PreparedStream
{
    VertexStream stream;
    std::shared_ptr<Graphics::Buffer> staging;
};

bool prepareStream(
    Graphics::Device &device,
    std::uint32_t slot,
    Graphics::VertexFormat format,
    std::uint32_t stride,
    const void *vertices,
    std::size_t size,
    PreparedStream &prepared,
    Error &error)
{
    Graphics::BufferDesc stagingDescription;
    stagingDescription.size = size;
    stagingDescription.usage = Graphics::BufferUsage::CopySource;
    stagingDescription.memoryType = Graphics::MemoryType::Upload;
    prepared.staging = device.CreateBuffer(stagingDescription, error);
    if (prepared.staging == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create mesh staging buffer.";
        }
        return false;
    }

    void *mapped = nullptr;
    if (!prepared.staging->Map(mapped, error)) {
        return false;
    }
    if (mapped == nullptr) {
        prepared.staging->Unmap();
        error.message = "The mesh staging buffer returned no mapped memory.";
        return false;
    }
    std::memcpy(mapped, vertices, size);
    prepared.staging->Unmap();

    Graphics::BufferDesc vertexDescription;
    vertexDescription.size = size;
    vertexDescription.usage =
        Graphics::BufferUsage::Vertex | Graphics::BufferUsage::CopyDestination;
    vertexDescription.memoryType = Graphics::MemoryType::DeviceLocal;
    prepared.stream.buffer = device.CreateBuffer(vertexDescription, error);
    if (prepared.stream.buffer == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create mesh vertex buffer.";
        }
        return false;
    }

    prepared.stream.slot = slot;
    prepared.stream.format = format;
    prepared.stream.stride = stride;
    prepared.stream.vertexCount = 3;
    return true;
}

bool recordCopy(
    Graphics::CommandContext &context,
    Graphics::CopyEncoder &encoder,
    const PreparedStream &stream,
    Error &error)
{
    Graphics::ResourceBarrierDesc sourceBarrier;
    sourceBarrier.before = Graphics::ResourceUsage::Undefined;
    sourceBarrier.after = Graphics::ResourceUsage::CopySource;
    if (!context.Barrier(stream.staging, sourceBarrier, error)) {
        return false;
    }

    Graphics::ResourceBarrierDesc destinationBarrier;
    destinationBarrier.before = Graphics::ResourceUsage::Undefined;
    destinationBarrier.after = Graphics::ResourceUsage::CopyDestination;
    if (!context.Barrier(stream.stream.buffer, destinationBarrier, error)) {
        return false;
    }
    if (!encoder.CopyBuffer(
            stream.staging, 0, stream.stream.buffer, 0,
            stream.staging->Description().size, error)) {
        return false;
    }

    destinationBarrier.before = Graphics::ResourceUsage::CopyDestination;
    destinationBarrier.after = Graphics::ResourceUsage::VertexBuffer;
    return context.Barrier(stream.stream.buffer, destinationBarrier, error);
}

} // namespace

bool MeshMaker::CreateUniformTriangle(Graphics::Device &device, Mesh &mesh, Error &error)
{
    static const float positions[] = {
        -0.5f, -0.28867513f, 0.0f,
         0.0f,  0.57735027f, 0.0f,
         0.5f, -0.28867513f, 0.0f,
    };
    static const float textureCoordinates[] = {
        0.0f, 1.0f,
        0.5f, 0.0f,
        1.0f, 1.0f,
    };

    PreparedStream positionStream;
    PreparedStream uvStream;
    Mesh generatedMesh;
    if (!prepareStream(
            device, 0, Graphics::VertexFormat::Float3, sizeof(float) * 3,
            positions, sizeof(positions), positionStream, error)
        || !prepareStream(
            device, 1, Graphics::VertexFormat::Float2, sizeof(float) * 2,
            textureCoordinates, sizeof(textureCoordinates), uvStream, error)) {
        return false;
    }
    if (!generatedMesh.AddVertexStream(positionStream.stream, error)
        || !generatedMesh.AddVertexStream(uvStream.stream, error)) {
        return false;
    }

    std::shared_ptr<Graphics::CommandQueue> queue =
        device.GetQueue(Graphics::QueueType::Graphics);
    if (queue == nullptr) {
        error.message = "The graphics device has no graphics command queue.";
        return false;
    }
    std::shared_ptr<Graphics::CommandContext> context = queue->CreateContext(error);
    if (context == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a command context for mesh upload.";
        }
        return false;
    }
    std::unique_ptr<Graphics::CopyEncoder> encoder = context->BeginCopy(error);
    if (encoder == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to begin a mesh upload copy encoder.";
        }
        return false;
    }
    if (!recordCopy(*context, *encoder, positionStream, error)
        || !recordCopy(*context, *encoder, uvStream, error)
        || !encoder->End(error)) {
        return false;
    }
    std::shared_ptr<Graphics::CommandBuffer> commandBuffer = context->Finish(error);
    if (commandBuffer == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to finish the mesh upload command buffer.";
        }
        return false;
    }
    std::shared_ptr<Graphics::Submission> submission =
        queue->Submit(commandBuffer, {}, {}, error);
    if (submission == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to submit the mesh upload.";
        }
        return false;
    }

    Mesh::PendingUpload pendingUpload;
    pendingUpload.submission = std::move(submission);
    pendingUpload.stagingBuffers = {
        std::move(positionStream.staging),
        std::move(uvStream.staging),
    };
    if (!generatedMesh.AddPendingUpload(std::move(pendingUpload), error)) {
        return false;
    }
    mesh = std::move(generatedMesh);
    return true;
}

} // namespace Engine
