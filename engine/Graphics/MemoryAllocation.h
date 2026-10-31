#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

#include <cstddef>

namespace Engine {
namespace Graphics {

class MemoryAllocation : public Resource
{
public:
    virtual std::size_t Size() const noexcept = 0;
    virtual MemoryType Type() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
