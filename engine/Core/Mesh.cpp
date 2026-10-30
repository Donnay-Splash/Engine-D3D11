#include "Mesh.h"

#include "Core/Types.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace Engine {
namespace {

std::uint32_t vertexFormatSize(Graphics::VertexFormat format)
{
    switch (format) {
    case Graphics::VertexFormat::Float2:
        return sizeof(float) * 2;
    case Graphics::VertexFormat::Float3:
        return sizeof(float) * 3;
    case Graphics::VertexFormat::Float4:
        return sizeof(float) * 4;
    case Graphics::VertexFormat::UInt32:
        return sizeof(std::uint32_t);
    }
    return 0;
}

} // namespace

bool Mesh::AddVertexStream(const VertexStream &stream, Error &error)
{
    const std::uint32_t formatSize = vertexFormatSize(stream.format);
    const std::uint32_t bufferUsage =
        stream.buffer == nullptr
            ? 0
            : static_cast<std::uint32_t>(stream.buffer->Description().usage);
    if (stream.buffer == nullptr || stream.vertexCount == 0 || stream.stride == 0
        || formatSize == 0 || stream.stride < formatSize
        || (bufferUsage & static_cast<std::uint32_t>(Graphics::BufferUsage::Vertex)) == 0
        || stream.vertexCount > std::numeric_limits<std::size_t>::max() / stream.stride
        || stream.buffer->Description().size
            < static_cast<std::size_t>(stream.vertexCount) * stream.stride) {
        error.message = "The mesh vertex stream has an invalid buffer layout.";
        return false;
    }
    if (!m_VertexStreams.empty() && stream.vertexCount != m_VertexStreams.front().vertexCount) {
        error.message = "All mesh vertex streams must have the same vertex count.";
        return false;
    }
    for (const VertexStream &existing : m_VertexStreams) {
        if (existing.slot == stream.slot) {
            error.message = "Mesh vertex stream slots must be unique.";
            return false;
        }
    }
    m_VertexStreams.push_back(stream);
    return true;
}

bool Mesh::AddPendingUpload(PendingUpload upload, Error &error)
{
    if (upload.submission == nullptr || upload.stagingBuffers.empty()) {
        error.message = "A pending mesh upload requires a submission and staging buffers.";
        return false;
    }
    if (std::any_of(upload.stagingBuffers.begin(), upload.stagingBuffers.end(),
            [](const std::shared_ptr<Graphics::Buffer> &buffer) { return buffer == nullptr; })) {
        error.message = "Pending mesh uploads cannot contain null staging buffers.";
        return false;
    }
    m_PendingUploads.push_back(std::move(upload));
    return true;
}

bool Mesh::CollectPendingUploads(
    std::vector<std::shared_ptr<Graphics::Submission>> &waitFor,
    Error &error)
{
    for (auto iterator = m_PendingUploads.begin(); iterator != m_PendingUploads.end();) {
        if (!iterator->submission->IsComplete()) {
            waitFor.push_back(iterator->submission);
            ++iterator;
            continue;
        }
        if (!iterator->submission->Wait(error)) {
            if (error.message.empty()) {
                error.message = "A mesh vertex upload failed.";
            }
            return false;
        }
        iterator = m_PendingUploads.erase(iterator);
    }
    return true;
}

const std::vector<VertexStream> &Mesh::VertexStreams() const noexcept
{
    return m_VertexStreams;
}

std::uint32_t Mesh::VertexCount() const noexcept
{
    return m_VertexStreams.empty() ? 0 : m_VertexStreams.front().vertexCount;
}

} // namespace Engine
