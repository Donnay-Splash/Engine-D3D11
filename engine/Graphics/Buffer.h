#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

#include <cstddef>

namespace Engine {
struct Error;
namespace Graphics {

class Buffer : public Resource
{
public:
    virtual const BufferDesc &Description() const noexcept = 0;
    virtual bool Map(void *&data, ::Engine::Error &error) = 0;
    virtual void Unmap() noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
