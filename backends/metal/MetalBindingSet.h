#pragma once

#include "MetalBackendCommon.h"

namespace Engine {
namespace Metal {

class MetalBindingSet final : public BindingSet
{
public:
    explicit MetalBindingSet(std::shared_ptr<BindingLayout> layout);

    std::shared_ptr<BindingLayout> Layout() const override;

    bool SetBuffer(std::uint32_t binding, std::uint32_t arrayElement,
                   const std::shared_ptr<BufferView> &view, Error &error) override;

    bool SetTexture(std::uint32_t binding, std::uint32_t arrayElement,
                    const std::shared_ptr<TextureView> &view, Error &error) override;

    bool SetSampler(std::uint32_t binding, std::uint32_t arrayElement,
                    const std::shared_ptr<Sampler> &sampler, Error &error) override;

    const std::vector<BindingValue> *Values(std::uint32_t binding) const;

private:
    const BindingDesc *Find(std::uint32_t binding, std::uint32_t arrayElement, Error &error) const;

    std::shared_ptr<BindingLayout> layout_;
    std::map<std::uint32_t, std::vector<BindingValue>> values_;
};

} // namespace Metal
} // namespace Engine
