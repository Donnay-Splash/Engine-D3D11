#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

namespace Engine {
namespace Graphics {

class Sampler : public Resource
{
public:
    virtual const SamplerDesc &Description() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
