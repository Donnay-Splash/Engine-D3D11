#pragma once

#include "Resource.h"

namespace Engine {
struct Error;
namespace Graphics {

class Submission : public Resource
{
public:
    virtual bool IsComplete() const = 0;
    virtual bool Wait(::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
