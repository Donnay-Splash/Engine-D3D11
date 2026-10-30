#pragma once

#include "Resource.h"

namespace Engine {
struct Error;
namespace Graphics {

class CommandBuffer : public Resource
{
public:
    // A submitted command buffer must retain every referenced resource until completion.
    virtual bool Reset(::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
