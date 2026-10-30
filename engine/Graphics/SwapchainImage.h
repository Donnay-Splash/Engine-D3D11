#pragma once

#include "Resource.h"

#include <memory>

namespace Engine {
namespace Graphics {

class TextureView;

class SwapchainImage : public Resource
{
public:
    virtual std::shared_ptr<TextureView> View() const = 0;
};

} // namespace Graphics
} // namespace Engine
