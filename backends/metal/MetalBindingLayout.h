#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalBindingLayout final : public BindingLayout
{
public:
    explicit MetalBindingLayout(std::vector<BindingDesc> bindings);

    const std::vector<BindingDesc> &Bindings() const noexcept override;

    const BindingDesc *Find(std::uint32_t binding) const;

private:
    std::vector<BindingDesc> bindings_;
};

} // namespace Metal
} // namespace Engine
