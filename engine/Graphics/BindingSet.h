#pragma once

#include "BindingLayout.h"
#include "BufferView.h"
#include "Resource.h"
#include "Sampler.h"
#include "TextureView.h"

#include <memory>

namespace Engine {
struct Error;
namespace Graphics {

class BindingSet : public Resource
{
public:
    virtual std::shared_ptr<BindingLayout> Layout() const = 0;
    virtual bool SetBuffer(
        std::uint32_t binding,
        std::uint32_t arrayElement,
        const std::shared_ptr<BufferView> &view,
        ::Engine::Error &error) = 0;
    virtual bool SetTexture(
        std::uint32_t binding,
        std::uint32_t arrayElement,
        const std::shared_ptr<TextureView> &view,
        ::Engine::Error &error) = 0;
    virtual bool SetSampler(
        std::uint32_t binding,
        std::uint32_t arrayElement,
        const std::shared_ptr<Sampler> &sampler,
        ::Engine::Error &error) = 0;
};

} // namespace Graphics
} // namespace Engine
