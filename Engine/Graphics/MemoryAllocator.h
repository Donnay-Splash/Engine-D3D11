#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

#include <cstddef>
#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class MemoryAllocation;

struct MemoryRequirements
{
    std::size_t size = 0;
    std::size_t alignment = 1;
};

class MemoryAllocator : public Resource
{
public:
    virtual MemoryRequirements GetRequirements(const BufferDesc &description) const = 0;
    virtual MemoryRequirements GetRequirements(const TextureDesc &description) const = 0;
    virtual std::shared_ptr<MemoryAllocation> Allocate(
        const MemoryRequirements &requirements,
        MemoryType type,
        ::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
