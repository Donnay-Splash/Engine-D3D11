#pragma once

#include "Buffer.h"
#include "Resource.h"

#include <cstddef>
#include <memory>

namespace Engine {
namespace Graphics {

class BufferView : public Resource
{
public:
    virtual std::shared_ptr<Buffer> GetBuffer() const = 0;
    virtual std::size_t Offset() const noexcept = 0;
    virtual std::size_t Size() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
