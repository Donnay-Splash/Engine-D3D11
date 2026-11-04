#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

// Register slots are only unique per kind of resource (b0, t0 and s0 can coexist).
enum class MetalResourceClass : std::uint32_t
{
    Buffer = 0,
    Texture = 1,
    Sampler = 2,
};

inline MetalResourceClass ResourceClassOf(BindingType type)
{
    switch (type) {
    case BindingType::SampledTexture:
    case BindingType::StorageTexture:
        return MetalResourceClass::Texture;
    case BindingType::Sampler:
        return MetalResourceClass::Sampler;
    default:
        return MetalResourceClass::Buffer;
    }
}

class MetalBindingLayout final : public BindingLayout
{
public:
    explicit MetalBindingLayout(std::vector<BindingDesc> bindings);

    const std::vector<BindingDesc> &Bindings() const noexcept override;

    const BindingDesc *Find(std::uint32_t binding, MetalResourceClass resourceClass) const;

private:
    std::vector<BindingDesc> bindings_;
};

} // namespace Metal
} // namespace Engine
