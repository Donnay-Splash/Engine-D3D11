#pragma once

#include "Core/Types.h"

namespace Engine {

class Renderer
{
public:
    virtual ~Renderer() = default;
    virtual bool Initialize(
        void *surface,
        Size size,
        const ErrorHandler &errorHandler,
        Error &error) = 0;
    virtual bool Resize(Size size, Error &error) = 0;
    virtual void Shutdown() noexcept = 0;
};

} // namespace Engine
