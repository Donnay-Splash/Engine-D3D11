#ifndef ENGINE_CORE_TYPES_H
#define ENGINE_CORE_TYPES_H

#include <cstdint>
#include <functional>
#include <string>

namespace Engine {

struct Error
{
    std::string message;
};

using ErrorHandler = std::function<void(const Error &)>;

struct Size
{
    std::uint32_t width;
    std::uint32_t height;
};

} // namespace Engine

#endif
