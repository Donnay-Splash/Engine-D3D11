#pragma once

#include "MetalBackendCommon.h"
#include "MetalBindingLayout.h"

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

    const std::vector<BindingValue> *Values(const BindingDesc &binding) const;

    // Metal has one top-level argument buffer for every set; each set's descriptors are
    // written at the offsets the shader reflection assigns to their (space, slot).
    // `sets` is indexed by set number; entries may be null for sets the pipeline does not use.
    static bool EncodeArgumentBuffer(MTL::Device *device,
                                     const std::vector<std::shared_ptr<MetalShader>> &shaders,
                                     const std::vector<std::shared_ptr<MetalBindingSet>> &sets,
                                     NS::SharedPtr<MTL::Buffer> &argumentBuffer, Error &error);

private:
    static std::uint64_t Key(std::uint32_t binding, MetalResourceClass resourceClass)
    {
        return (static_cast<std::uint64_t>(resourceClass) << 32) | binding;
    }

    const BindingDesc *Find(std::uint32_t binding, MetalResourceClass resourceClass,
                            std::uint32_t arrayElement, Error &error) const;

    std::shared_ptr<BindingLayout> layout_;
    std::map<std::uint64_t, std::vector<BindingValue>> values_;
};

} // namespace Metal
} // namespace Engine
