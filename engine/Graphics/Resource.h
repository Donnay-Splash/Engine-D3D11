#pragma once

namespace Engine {
namespace Graphics {

class Resource
{
public:
    Resource(const Resource &) = delete;
    Resource &operator=(const Resource &) = delete;
    // Backends must defer native destruction until every submission using this resource completes.
    virtual ~Resource() = default;

protected:
    Resource() = default;
};

} // namespace Graphics
} // namespace Engine
