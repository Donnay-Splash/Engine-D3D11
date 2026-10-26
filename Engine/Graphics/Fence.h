#pragma once

#include "Resource.h"

namespace Engine {
struct Error;
namespace Graphics {

class Fence : public Resource
{
public:
    virtual bool IsSignaled() const = 0;
    virtual bool Wait(::Engine::Error &error) = 0;
    virtual bool Reset(::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
