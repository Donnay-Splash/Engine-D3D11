#include "Rendering/ShaderProgram.h"

namespace Engine {

std::optional<Graphics::BindingDesc> ShaderProgram::FindBinding(Graphics::BindingType type) const
{
    const std::shared_ptr<Graphics::BindingLayout> layout = BindingLayout(0);
    if (layout == nullptr) {
        return std::nullopt;
    }
    for (const Graphics::BindingDesc &binding : layout->Bindings()) {
        if (binding.type == type) {
            return binding;
        }
    }
    return std::nullopt;
}

} // namespace Engine
