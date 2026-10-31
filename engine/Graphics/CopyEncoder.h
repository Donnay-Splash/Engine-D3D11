#pragma once

#include "CommandEncoder.h"

#include <cstddef>
#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class Buffer;

class CopyEncoder : public CommandEncoder
{
public:
    virtual bool CopyBuffer(
        const std::shared_ptr<Buffer> &source,
        std::size_t sourceOffset,
        const std::shared_ptr<Buffer> &destination,
        std::size_t destinationOffset,
        std::size_t size,
        ::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
