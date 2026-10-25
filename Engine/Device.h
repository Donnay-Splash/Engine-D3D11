#pragma once

#include "Engine.h"

namespace Engine {

class Device
{
public:
    virtual ~Device() {}
    virtual bool Initialize(
        void *surface,
        Size size,
        const ErrorHandler &errorHandler,
        Error &error) = 0;
    virtual bool Resize(Size size, Error &error) = 0;
    virtual void Shutdown() noexcept = 0;
};

class EngineFactory
{
public:
    static std::unique_ptr<Engine> create(std::unique_ptr<Device> device);
};

} // namespace Engine
