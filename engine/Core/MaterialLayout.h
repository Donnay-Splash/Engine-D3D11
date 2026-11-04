#pragma once

#include "Core/MaterialValue.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/ShaderReflection.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Engine {

struct Error;

struct MaterialParameter
{
    std::string name;
    MaterialValueType type = MaterialValueType::Float;
    // Byte offset in the parameter buffer; unused for textures.
    std::uint32_t offset = 0;
    // Texture binding in the material set; unused for buffer values.
    std::uint32_t binding = 0;
    MaterialValue defaultValue;
    // What an unset texture parameter shows; unused for buffer values.
    DefaultTexture defaultTexture = DefaultTexture::White;
};

// The parameters a shader program exposes to materials, derived from reflection: the
// scalar and vector members of the one constant buffer in the material set, plus its
// sampled textures. Built once per program; materials and instances only refer to it.
class MaterialLayout
{
public:
    // Register space reserved for material resources.
    static constexpr std::uint32_t kMaterialSet = 1;

    // Merges the material set of every stage of a program. Stages that declare the same
    // resource must agree on its layout. `layout` is null when the program has no material
    // resources.
    static bool Build(
        const std::vector<const Graphics::ShaderReflection *> &stages,
        std::shared_ptr<const MaterialLayout> &layout,
        Error &error);

    const std::vector<MaterialParameter> &Parameters() const noexcept { return m_Parameters; }
    std::optional<std::size_t> Find(const std::string &name) const noexcept;

    // Size in bytes of the parameter buffer; zero when there are only textures.
    std::uint32_t BufferSize() const noexcept { return m_BufferSize; }
    std::uint32_t BufferBinding() const noexcept { return m_BufferBinding; }
    bool HasBuffer() const noexcept { return m_BufferSize != 0; }

    // One value per parameter in order. Writes buffer parameters at their offsets
    // (bool as a 32-bit integer) and ignores textures.
    std::vector<std::uint8_t> Pack(const std::vector<MaterialValue> &values) const;

private:
    std::vector<MaterialParameter> m_Parameters;
    std::uint32_t m_BufferSize = 0;
    std::uint32_t m_BufferBinding = 0;
};

// The zero value of a type, used when a shader declares no default.
MaterialValue DefaultValueFor(MaterialValueType type);

} // namespace Engine
