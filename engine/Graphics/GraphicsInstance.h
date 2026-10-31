#pragma once

#include "Resource.h"

#include <memory>
#include <vector>

namespace Engine {
struct Error;
namespace Graphics {

class Adapter;

class GraphicsInstance : public Resource
{
public:
    virtual bool EnumerateAdapters(
        std::vector<std::shared_ptr<Adapter>> &adapters,
        ::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
