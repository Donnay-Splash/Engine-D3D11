#pragma once

#include "BindingLayout.h"
#include "Resource.h"

#include <memory>

namespace Engine {
namespace Graphics {

class Pipeline : public Resource
{
public:
    virtual std::shared_ptr<BindingLayout> Layout() const = 0;
};

} // namespace Graphics
} // namespace Engine
