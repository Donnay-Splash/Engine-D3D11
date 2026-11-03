#include "Rendering/ShaderProgram.h"

namespace Engine {

std::optional<Graphics::BindingDesc> ShaderProgram::FindBinding(Graphics::BindingType type) const
{
    if (pipelineTemplate.bindingLayout == nullptr) {
        return std::nullopt;
    }
    for (const Graphics::BindingDesc &binding : pipelineTemplate.bindingLayout->Bindings()) {
        if (binding.type == type) {
            return binding;
        }
    }
    return std::nullopt;
}

} // namespace Engine
