#pragma once

#include "Pipeline.h"
#include "Shader.h"

#include <memory>

namespace Engine {
namespace Graphics {

struct ComputePipelineDesc
{
    std::shared_ptr<Shader> shader;
};

class ComputePipeline : public Pipeline
{
public:
    virtual const ComputePipelineDesc &Description() const noexcept = 0;
};

} // namespace Graphics
} // namespace Engine
