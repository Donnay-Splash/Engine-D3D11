#pragma once

#include "Graphics/Buffer.h"
#include "Graphics/Submission.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Engine {

struct Error;

struct VertexStream
{
    std::uint32_t slot = 0;
    Graphics::VertexFormat format = Graphics::VertexFormat::Float3;
    std::uint32_t stride = 0;
    std::uint32_t vertexCount = 0;
    std::shared_ptr<Graphics::Buffer> buffer;
};

class Mesh
{
public:
    struct PendingUpload
    {
        std::shared_ptr<Graphics::Submission> submission;
        std::vector<std::shared_ptr<Graphics::Buffer>> stagingBuffers;
    };

    bool AddVertexStream(const VertexStream &stream, Error &error);
    bool AddPendingUpload(PendingUpload upload, Error &error);
    bool CollectPendingUploads(
        std::vector<std::shared_ptr<Graphics::Submission>> &waitFor,
        Error &error);

    const std::vector<VertexStream> &VertexStreams() const noexcept;
    std::uint32_t VertexCount() const noexcept;

private:
    std::vector<VertexStream> m_VertexStreams;
    std::vector<PendingUpload> m_PendingUploads;
};

} // namespace Engine
