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

const BindingDesc *MetalBindingLayout::Find(std::uint32_t binding,
                                            MetalResourceClass resourceClass) const
{
    const auto found = std::find_if(
        bindings_.begin(), bindings_.end(), [binding, resourceClass](const BindingDesc &item) {
            return item.binding == binding && ResourceClassOf(item.type) == resourceClass;
        });
    return found == bindings_.end() ? nullptr : &*found;
}

} // namespace Metal
} // namespace Engine
