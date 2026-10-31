#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalBindingLayout::MetalBindingLayout(std::vector<BindingDesc> bindings)
    : bindings_(std::move(bindings))
{
}

const std::vector<BindingDesc> &MetalBindingLayout::Bindings() const noexcept
{
    return bindings_;
}

const BindingDesc *MetalBindingLayout::Find(std::uint32_t binding) const
{
    const auto found =
        std::find_if(bindings_.begin(), bindings_.end(),
                     [binding](const BindingDesc &item) { return item.binding == binding; });
    return found == bindings_.end() ? nullptr : &*found;
}

} // namespace Metal
} // namespace Engine
