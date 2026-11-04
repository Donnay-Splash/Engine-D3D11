#include "MetalBackendInternal.h"

namespace Engine {
namespace Metal {

using namespace Graphics;

namespace {

struct ResourceDescriptor
{
    std::uint64_t gpuAddress = 0;
    std::uint64_t resourceID = 0;
    std::uint64_t metadata = 0;
};

static_assert(sizeof(ResourceDescriptor) == 3 * sizeof(std::uint64_t));

bool MatchesType(const BindingDesc &binding, const std::string &type)
{
    switch (binding.type) {
    case BindingType::UniformBuffer:
        return type == "CBV";
    case BindingType::StorageBuffer:
        return type == "SRV" || type == "UAV";
    case BindingType::StorageTexture:
        return type == "UAV";
    case BindingType::SampledTexture:
        return type == "SRV";
    case BindingType::Sampler:
        return type == "SMP" || type == "Sampler";
    }
    return false;
}

const MetalBindingLocation *FindLocation(const MetalShader &shader, const BindingDesc &binding,
                                         Error &error)
{
    const MetalBindingLocation *match = nullptr;
    for (const MetalBindingLocation &location : shader.BindingLocations()) {
        if (location.slot != binding.binding || location.space != binding.set ||
            !MatchesType(binding, location.type)) {
            continue;
        }
        if (match != nullptr) {
            SetError(error, "The Metal shader reflection maps multiple resources to one binding.");
            return nullptr;
        }
        match = &location;
    }
    if (match == nullptr) {
        SetError(error, "The Metal shader reflection is missing a declared resource binding.");
    }
    return match;
}

bool EncodeDescriptor(const BindingDesc &binding, const BindingValue &value,
                      ResourceDescriptor &descriptor, Error &error)
{
    if (binding.type == BindingType::UniformBuffer || binding.type == BindingType::StorageBuffer) {
        auto view = std::dynamic_pointer_cast<MetalBufferView>(value.buffer);
        if (view == nullptr || view->NativeBuffer() == nullptr ||
            view->Size() > std::numeric_limits<std::uint32_t>::max()) {
            SetError(error, "A buffer argument-buffer binding is unset or too large.");
            return false;
        }
        const MTL::GPUAddress baseAddress = view->NativeBuffer()->Native()->gpuAddress();
        if (baseAddress == 0 ||
            view->Offset() > std::numeric_limits<MTL::GPUAddress>::max() - baseAddress) {
            SetError(error, "The Metal buffer has no valid GPU address.");
            return false;
        }
        descriptor.gpuAddress = baseAddress + view->Offset();
        descriptor.metadata = static_cast<std::uint32_t>(view->Size());
        return true;
    }
    if (binding.type == BindingType::SampledTexture ||
        binding.type == BindingType::StorageTexture) {
        auto view = std::dynamic_pointer_cast<MetalTextureView>(value.texture);
        if (view == nullptr || view->Native() == nullptr) {
            SetError(error, "A texture argument-buffer binding is unset.");
            return false;
        }
        descriptor.resourceID = view->Native()->gpuResourceID()._impl;
        return descriptor.resourceID != 0;
    }
    auto sampler = std::dynamic_pointer_cast<MetalSampler>(value.sampler);
    if (sampler == nullptr || sampler->Native() == nullptr) {
        SetError(error, "A sampler argument-buffer binding is unset.");
        return false;
    }
    descriptor.gpuAddress = sampler->Native()->gpuResourceID()._impl;
    return descriptor.gpuAddress != 0;
}

} // namespace

MetalBindingSet::MetalBindingSet(std::shared_ptr<BindingLayout> layout) : layout_(std::move(layout))
{
    for (const BindingDesc &binding : layout_->Bindings()) {
        values_[Key(binding.binding, ResourceClassOf(binding.type))].resize(binding.arrayCount);
    }
}

std::shared_ptr<BindingLayout> MetalBindingSet::Layout() const
{
    return layout_;
}

bool MetalBindingSet::SetBuffer(std::uint32_t binding, std::uint32_t arrayElement,
                                const std::shared_ptr<BufferView> &view, Error &error)
{
    const BindingDesc *description =
        Find(binding, MetalResourceClass::Buffer, arrayElement, error);
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
    values_[Key(binding, MetalResourceClass::Buffer)][arrayElement].buffer = view;
    return true;
}

bool MetalBindingSet::SetTexture(std::uint32_t binding, std::uint32_t arrayElement,
                                 const std::shared_ptr<TextureView> &view, Error &error)
{
    const BindingDesc *description =
        Find(binding, MetalResourceClass::Texture, arrayElement, error);
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
    values_[Key(binding, MetalResourceClass::Texture)][arrayElement].texture = view;
    return true;
}

bool MetalBindingSet::SetSampler(std::uint32_t binding, std::uint32_t arrayElement,
                                 const std::shared_ptr<Sampler> &sampler, Error &error)
{
    const BindingDesc *description =
        Find(binding, MetalResourceClass::Sampler, arrayElement, error);
    if (description == nullptr) {
        return false;
    }
    if (description->type != BindingType::Sampler ||
        dynamic_cast<MetalSampler *>(sampler.get()) == nullptr) {
        SetError(error, "The binding and sampler do not match.");
        return false;
    }
    values_[Key(binding, MetalResourceClass::Sampler)][arrayElement].sampler = sampler;
    return true;
}

const std::vector<BindingValue> *MetalBindingSet::Values(const BindingDesc &binding) const
{
    const auto found = values_.find(Key(binding.binding, ResourceClassOf(binding.type)));
    return found == values_.end() ? nullptr : &found->second;
}

bool MetalBindingSet::EncodeArgumentBuffer(
    MTL::Device *device, const std::vector<std::shared_ptr<MetalShader>> &shaders,
    const std::vector<std::shared_ptr<MetalBindingSet>> &sets,
    NS::SharedPtr<MTL::Buffer> &argumentBuffer, Error &error)
{
    // Calls fn(set, binding) for each layout binding of every bound set visible to `stage`.
    const auto forEachBinding = [&sets](std::uint32_t stage, auto &&fn) {
        for (const std::shared_ptr<MetalBindingSet> &set : sets) {
            if (set == nullptr) {
                continue;
            }
            for (const BindingDesc &binding : set->layout_->Bindings()) {
                if ((static_cast<std::uint32_t>(binding.visibility) & stage) != 0 &&
                    !fn(*set, binding)) {
                    return false;
                }
            }
        }
        return true;
    };

    std::size_t argumentBufferSize = 0;
    for (const std::shared_ptr<MetalShader> &shader : shaders) {
        if (shader == nullptr) {
            SetError(error, "The Metal pipeline has an invalid shader.");
            return false;
        }
        const std::uint32_t stage = static_cast<std::uint32_t>(shader->Stage());
        const bool ranges = forEachBinding(stage, [&](const MetalBindingSet &, const BindingDesc &binding) {
            const MetalBindingLocation *location = FindLocation(*shader, binding, error);
            if (location == nullptr) {
                return false;
            }
            std::size_t requiredSize = 0;
            if (!CheckedMultiply(binding.arrayCount, sizeof(ResourceDescriptor), requiredSize) ||
                location->offset % alignof(ResourceDescriptor) != 0 ||
                location->size < requiredSize ||
                location->offset > std::numeric_limits<std::size_t>::max() - requiredSize) {
                SetError(error, "The Metal shader reflection has an incompatible binding range.");
                return false;
            }
            argumentBufferSize = std::max(argumentBufferSize, location->offset + requiredSize);
            return true;
        });
        if (!ranges) {
            return false;
        }
        for (const MetalBindingLocation &location : shader->BindingLocations()) {
            const bool covered = !forEachBinding(
                stage, [&](const MetalBindingSet &, const BindingDesc &binding) {
                    return !(binding.binding == location.slot && binding.set == location.space &&
                             MatchesType(binding, location.type));
                });
            if (!covered) {
                SetError(error, "The Metal binding layout does not cover every reflected shader resource.");
                return false;
            }
        }
    }
    if (argumentBufferSize == 0) {
        argumentBufferSize = sizeof(ResourceDescriptor);
    }
    MTL::Buffer *nativeBuffer =
        device->newBuffer(argumentBufferSize, MTL::ResourceStorageModeShared);
    if (nativeBuffer == nullptr || nativeBuffer->contents() == nullptr) {
        if (nativeBuffer != nullptr) {
            nativeBuffer->release();
        }
        SetError(error, "Metal failed to allocate a CPU-visible argument buffer.");
        return false;
    }
    argumentBuffer = NS::TransferPtr(nativeBuffer);
    std::memset(argumentBuffer->contents(), 0, argumentBufferSize);

    for (const std::shared_ptr<MetalShader> &shader : shaders) {
        const std::uint32_t stage = static_cast<std::uint32_t>(shader->Stage());
        const bool written = forEachBinding(stage, [&](const MetalBindingSet &set, const BindingDesc &binding) {
            const MetalBindingLocation *location = FindLocation(*shader, binding, error);
            const std::vector<BindingValue> *values = set.Values(binding);
            if (location == nullptr || values == nullptr) {
                return false;
            }
            for (std::uint32_t element = 0; element < binding.arrayCount; ++element) {
                ResourceDescriptor descriptor;
                if (!EncodeDescriptor(binding, (*values)[element], descriptor, error)) {
                    return false;
                }
                const std::size_t offset =
                    location->offset + element * sizeof(ResourceDescriptor);
                std::memcpy(static_cast<std::uint8_t *>(argumentBuffer->contents()) + offset,
                            &descriptor, sizeof(descriptor));
            }
            return true;
        });
        if (!written) {
            return false;
        }
    }
    return true;
}

const BindingDesc *MetalBindingSet::Find(std::uint32_t binding, MetalResourceClass resourceClass,
                                         std::uint32_t arrayElement, Error &error) const
{
    auto *layout = dynamic_cast<MetalBindingLayout *>(layout_.get());
    const BindingDesc *description =
        layout == nullptr ? nullptr : layout->Find(binding, resourceClass);
    if (description == nullptr || arrayElement >= description->arrayCount) {
        SetError(error, "The binding index or array element is outside the layout.");
        return nullptr;
    }
    return description;
}

} // namespace Metal
} // namespace Engine
