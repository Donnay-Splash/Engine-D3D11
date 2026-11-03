#pragma once

#include "Graphics/BindingLayout.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/ShaderReflection.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Engine {

// A resolved graphics program: its shaders plus everything derivable from their reflection.
// Only the state that depends on the mesh and the render target is left for PipelineCache.
struct ShaderProgram
{
    std::string name;
    // Shaders and binding layout are already filled in.
    Graphics::GraphicsPipelineDesc pipelineTemplate;
    std::vector<Graphics::ShaderVertexInput> vertexInputs;

    std::shared_ptr<Graphics::BindingLayout> BindingLayout() const
    {
        return pipelineTemplate.bindingLayout;
    }

    // First reflected binding of the given type, if the program has one.
    std::optional<Graphics::BindingDesc> FindBinding(Graphics::BindingType type) const;
};

} // namespace Engine
