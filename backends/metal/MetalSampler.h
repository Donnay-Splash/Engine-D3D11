#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalSampler final : public Sampler
{
public:
    MetalSampler(SamplerDesc description, MTL::SamplerState *sampler);

    const SamplerDesc &Description() const noexcept override;

    MTL::SamplerState *Native() const noexcept;

private:
    SamplerDesc description_;
    NS::SharedPtr<MTL::SamplerState> sampler_;
};

} // namespace Metal
} // namespace Engine
