#include "Rendering/MaterialBinder.h"

#include "Graphics/BufferView.h"
#include "Graphics/CommandContext.h"
#include "Graphics/Device.h"
#include "Rendering/ShaderProgram.h"

#include <cstring>

namespace Engine {

namespace {

// Keeps uniform views a multiple of the 16 byte constant register size.
std::size_t alignedBufferSize(std::uint32_t size)
{
    return (static_cast<std::size_t>(size) + 15u) & ~static_cast<std::size_t>(15u);
}

} // namespace

bool MaterialBinder::Initialize(
    std::shared_ptr<Graphics::Device> device,
    std::shared_ptr<Graphics::TextureView> fallbackTexture,
    Error &error)
{
    if (device == nullptr) {
        error.message = "The material binder requires a device.";
        return false;
    }
    m_Device = std::move(device);
    m_FallbackTexture = std::move(fallbackTexture);
    return true;
}

void MaterialBinder::Shutdown() noexcept
{
    m_Entries.clear();
    m_FallbackTexture.reset();
    m_Device.reset();
}

void MaterialBinder::BeginFrame()
{
    ++m_Frame;
    for (auto it = m_Entries.begin(); it != m_Entries.end();) {
        it = it->second.lastUsedFrame + 1 < m_Frame ? m_Entries.erase(it) : std::next(it);
    }
}

bool MaterialBinder::CreateEntry(
    const ShaderProgram &program,
    const MaterialLayout &layout,
    Graphics::CommandContext &context,
    Entry &entry,
    Error &error)
{
    if (layout.HasBuffer()) {
        Graphics::BufferDesc description;
        description.size = alignedBufferSize(layout.BufferSize());
        description.usage = Graphics::BufferUsage::Uniform;
        description.memoryType = Graphics::MemoryType::Upload;
        entry.buffer = m_Device->CreateBuffer(description, error);
        if (entry.buffer == nullptr) {
            if (error.message.empty()) {
                error.message = "Failed to create a material uniform buffer.";
            }
            return false;
        }
        entry.bufferView = m_Device->CreateBufferView(entry.buffer, 0, description.size, error);
        if (entry.bufferView == nullptr) {
            if (error.message.empty()) {
                error.message = "Failed to create a view of a material uniform buffer.";
            }
            return false;
        }
        Graphics::ResourceBarrierDesc barrier;
        barrier.before = Graphics::ResourceUsage::Undefined;
        barrier.after = Graphics::ResourceUsage::UniformBuffer;
        if (!context.Barrier(entry.buffer, barrier, error)) {
            if (error.message.empty()) {
                error.message = "Failed to prepare a material uniform buffer.";
            }
            return false;
        }
    }
    entry.bindings = m_Device->CreateBindingSet(
        program.BindingLayout(MaterialLayout::kMaterialSet), error);
    if (entry.bindings == nullptr) {
        if (error.message.empty()) {
            error.message = "Failed to create a material binding set.";
        }
        return false;
    }
    if (entry.bufferView != nullptr
        && !entry.bindings->SetBuffer(layout.BufferBinding(), 0, entry.bufferView, error)) {
        if (error.message.empty()) {
            error.message = "Failed to bind the material uniform buffer.";
        }
        return false;
    }
    entry.layout = program.materialLayout;
    return true;
}

bool MaterialBinder::Update(
    const MaterialInstance &instance,
    const MaterialLayout &layout,
    Entry &entry,
    Error &error)
{
    const std::vector<MaterialValue> values = instance.ResolveValues();
    if (entry.buffer != nullptr) {
        const std::vector<std::uint8_t> bytes = layout.Pack(values);
        void *mapped = nullptr;
        if (!entry.buffer->Map(mapped, error)) {
            return false;
        }
        if (mapped == nullptr) {
            entry.buffer->Unmap();
            error.message = "The material uniform buffer returned no mapped memory.";
            return false;
        }
        std::memcpy(mapped, bytes.data(), bytes.size());
        entry.buffer->Unmap();
    }
    entry.textures.clear();
    for (std::size_t index = 0; index < values.size(); ++index) {
        const MaterialParameter &parameter = layout.Parameters()[index];
        if (parameter.type != MaterialValueType::Texture) {
            continue;
        }
        MaterialTexture texture = std::get<MaterialTexture>(values[index]);
        if (texture == nullptr) {
            texture = m_FallbackTexture;
        }
        if (texture == nullptr) {
            error.message = "Material texture '" + parameter.name
                + "' is not set and there is no default texture.";
            return false;
        }
        if (!entry.bindings->SetTexture(parameter.binding, 0, texture, error)) {
            if (error.message.empty()) {
                error.message = "Failed to bind material texture '" + parameter.name + "'.";
            }
            return false;
        }
        entry.textures.push_back(std::move(texture));
    }
    entry.version = instance.Version();
    return true;
}

std::shared_ptr<Graphics::BindingSet> MaterialBinder::Acquire(
    const MaterialInstance &instance,
    const ShaderProgram &program,
    Graphics::CommandContext &context,
    Error &error)
{
    const std::shared_ptr<const MaterialLayout> &layout = program.materialLayout;
    if (layout == nullptr) {
        return nullptr;
    }
    if (instance.GetMaterial()->Layout() != layout) {
        error.message = "Material '" + instance.GetMaterial()->Name()
            + "' was not created from the layout of shader program '" + program.name + "'.";
        return nullptr;
    }
    Entry &entry = m_Entries[&instance];
    entry.lastUsedFrame = m_Frame;
    if (entry.layout != layout) {
        entry = Entry{};
        entry.lastUsedFrame = m_Frame;
        if (!CreateEntry(program, *layout, context, entry, error)) {
            m_Entries.erase(&instance);
            return nullptr;
        }
    }
    if (entry.version != instance.Version() && !Update(instance, *layout, entry, error)) {
        m_Entries.erase(&instance);
        return nullptr;
    }
    return entry.bindings;
}

} // namespace Engine
