#pragma once

#include "GraphicsTypes.h"
#include "Resource.h"

#include <vector>

namespace Engine {
struct Error;
namespace Graphics {

class BindingLayout : public Resource
{
public:
    virtual const std::vector<BindingDesc> &Bindings() const noexcept = 0;
};

struct BindingLayoutDesc
{
    std::vector<BindingDesc> bindings;
};

} // namespace Graphics
} // namespace Engine
