#pragma once

#include "Core/MaterialLayout.h"
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
    // Parameters materials can set on this program; null when it has none.
    std::shared_ptr<const MaterialLayout> materialLayout;

    // Layout of a binding set, or null when the program has no bindings in it.
    std::shared_ptr<Graphics::BindingLayout> BindingLayout(std::uint32_t set = 0) const
    {
        const auto &layouts = pipelineTemplate.bindingLayouts;
        return set < layouts.size() ? layouts[set] : nullptr;
    }

    // First reflected binding of the given type in the engine set, if the program has one.
    std::optional<Graphics::BindingDesc> FindBinding(Graphics::BindingType type) const;
};

} // namespace Engine
