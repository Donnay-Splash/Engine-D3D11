#pragma once

#include "Core/Types.h"

#include <memory>
#include <string>
#include <vector>

namespace Engine {
namespace Graphics {
class Device;
class Sampler;
} // namespace Graphics

// The fixed set of samplers the engine provides to every shader through set 0. Shaders
// pick one by declaring a sampler with one of these names; materials do not own samplers.
class EngineSamplers
{
public:
    bool Initialize(Graphics::Device &device, Error &error);
    void Shutdown() noexcept;

    // Null when `name` is not an engine sampler.
    std::shared_ptr<Graphics::Sampler> Find(const std::string &name) const;

private:
    struct Entry
    {
        std::string name;
        std::shared_ptr<Graphics::Sampler> sampler;
    };

    std::vector<Entry> m_Samplers;
};

} // namespace Engine
