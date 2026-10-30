#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

MetalBindingSet::MetalBindingSet(std::shared_ptr<BindingLayout> layout) : layout_(std::move(layout))
{
    for (const BindingDesc &binding : layout_->Bindings()) {
        values_[binding.binding].resize(binding.arrayCount);
    }
}

std::shared_ptr<BindingLayout> MetalBindingSet::Layout() const
{
    return layout_;
}

bool MetalBindingSet::SetBuffer(std::uint32_t binding, std::uint32_t arrayElement,
                                const std::shared_ptr<BufferView> &view, Error &error)
{
    const BindingDesc *description = Find(binding, arrayElement, error);
    if (description == nullptr) {
        return false;
    }
    if (description->type != BindingType::UniformBuffer &&
        description->type != BindingType::StorageBuffer) {
        SetError(error, "The binding is not declared as a buffer.");
        return false;
    }
    auto *metalView = dynamic_cast<MetalBufferView *>(view.get());
    if (metalView == nullptr || metalView->NativeBuffer() == nullptr) {
        SetError(error, "The buffer view is not owned by the Metal backend.");
        return false;
    }
    const BufferUsage usage = metalView->GetBuffer()->Description().usage;
    const std::uint32_t required = description->type == BindingType::UniformBuffer
                                       ? static_cast<std::uint32_t>(BufferUsage::Uniform)
                                       : static_cast<std::uint32_t>(BufferUsage::Storage);
    if (!HasBits(static_cast<std::uint32_t>(usage), required)) {
        SetError(error, "The buffer usage does not match the binding declaration.");
        return false;
    }
    values_[binding][arrayElement].buffer = view;
    return true;
}

bool MetalBindingSet::SetTexture(std::uint32_t binding, std::uint32_t arrayElement,
                                 const std::shared_ptr<TextureView> &view, Error &error)
{
    const BindingDesc *description = Find(binding, arrayElement, error);
    if (description == nullptr) {
        return false;
    }
    if (description->type != BindingType::SampledTexture &&
        description->type != BindingType::StorageTexture) {
        SetError(error, "The binding is not declared as a texture.");
        return false;
    }
    auto *metalView = dynamic_cast<MetalTextureView *>(view.get());
    if (metalView == nullptr) {
        SetError(error, "The texture view is not owned by the Metal backend.");
        return false;
    }
    const TextureUsage usage = metalView->GetTexture()->Description().usage;
    const std::uint32_t required = description->type == BindingType::SampledTexture
                                       ? static_cast<std::uint32_t>(TextureUsage::Sampled)
                                       : static_cast<std::uint32_t>(TextureUsage::Storage);
    if (!HasBits(static_cast<std::uint32_t>(usage), required)) {
        SetError(error, "The texture usage does not match the binding declaration.");
        return false;
    }
    values_[binding][arrayElement].texture = view;
    return true;
}

bool MetalBindingSet::SetSampler(std::uint32_t binding, std::uint32_t arrayElement,
                                 const std::shared_ptr<Sampler> &sampler, Error &error)
{
    const BindingDesc *description = Find(binding, arrayElement, error);
    if (description == nullptr) {
        return false;
    }
    if (description->type != BindingType::Sampler ||
        dynamic_cast<MetalSampler *>(sampler.get()) == nullptr) {
        SetError(error, "The binding and sampler do not match.");
        return false;
    }
    values_[binding][arrayElement].sampler = sampler;
    return true;
}

const std::vector<BindingValue> *MetalBindingSet::Values(std::uint32_t binding) const
{
    const auto found = values_.find(binding);
    return found == values_.end() ? nullptr : &found->second;
}

const BindingDesc *MetalBindingSet::Find(std::uint32_t binding, std::uint32_t arrayElement,
                                         Error &error) const
{
    auto *layout = dynamic_cast<MetalBindingLayout *>(layout_.get());
    const BindingDesc *description = layout == nullptr ? nullptr : layout->Find(binding);
    if (description == nullptr || arrayElement >= description->arrayCount) {
        SetError(error, "The binding index or array element is outside the layout.");
        return nullptr;
    }
    return description;
}

} // namespace Metal
} // namespace Engine
