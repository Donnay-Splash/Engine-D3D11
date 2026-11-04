#pragma once

#include "Core/MaterialInstance.h"
#include "Core/Types.h"
#include "Graphics/BindingSet.h"
#include "Graphics/Buffer.h"

#include <cstdint>
#include <map>
#include <memory>

namespace Engine {
namespace Graphics {
class CommandContext;
class Device;
} // namespace Graphics

class CommonTextures;
struct ShaderProgram;

// Owns the GPU side of material instances: a uniform buffer and a material-set binding set
// per instance, rewritten when the instance changes. Writes go straight to upload memory,
// so callers must only use it while the GPU is not reading the previous frame.
class MaterialBinder
{
public:
    // Texture parameters that are not set get their default common texture, so no descriptor
    // is ever left unbound. `commonTextures` must outlive the binder.
    bool Initialize(
        std::shared_ptr<Graphics::Device> device,
        const CommonTextures &commonTextures,
        Error &error);
    void Shutdown() noexcept;

    // Instances not acquired during the previous frame are released.
    void BeginFrame();

    // Returns the binding set for the instance's material resources, updating it if the
    // instance changed. Null (without an error) when the program has no material resources.
    std::shared_ptr<Graphics::BindingSet> Acquire(
        const MaterialInstance &instance,
        const ShaderProgram &program,
        Graphics::CommandContext &context,
        Error &error);

private:
    struct Entry
    {
        std::shared_ptr<Graphics::Buffer> buffer;
        std::shared_ptr<Graphics::BufferView> bufferView;
        std::shared_ptr<Graphics::BindingSet> bindings;
        std::shared_ptr<const MaterialLayout> layout;
        std::vector<MaterialTexture> textures; // keeps bound textures alive
        std::uint64_t version = 0;
        std::uint64_t lastUsedFrame = 0;
    };

    bool CreateEntry(
        const ShaderProgram &program,
        const MaterialLayout &layout,
        Graphics::CommandContext &context,
        Entry &entry,
        Error &error);
    MaterialTexture DefaultTextureFor(DefaultTexture texture) const;
    bool Update(const MaterialInstance &instance, const MaterialLayout &layout, Entry &entry, Error &error);

    std::shared_ptr<Graphics::Device> m_Device;
    const CommonTextures *m_CommonTextures = nullptr;
    std::map<const MaterialInstance *, Entry> m_Entries;
    std::uint64_t m_Frame = 0;
};

} // namespace Engine
