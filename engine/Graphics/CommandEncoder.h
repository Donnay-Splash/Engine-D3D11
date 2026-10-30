#pragma once

namespace Engine {
struct Error;
namespace Graphics {

class CommandEncoder
{
public:
    virtual ~CommandEncoder() = default;
    virtual bool End(::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
