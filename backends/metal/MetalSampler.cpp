#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalSampler::MetalSampler(SamplerDesc description, MTL::SamplerState *sampler)
    : description_(description), sampler_(NS::RetainPtr(sampler))
{
}

const SamplerDesc &MetalSampler::Description() const noexcept
{
    return description_;
}

MTL::SamplerState *MetalSampler::Native() const noexcept
{
    return sampler_.get();
}

} // namespace Metal
} // namespace Engine
