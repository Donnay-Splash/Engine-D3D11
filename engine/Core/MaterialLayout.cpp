#include "Core/MaterialLayout.h"

#include "Core/Types.h"

#include <algorithm>

namespace Engine {
namespace {

MaterialValueType toMaterialType(Graphics::ShaderValueType type)
{
    switch (type) {
    case Graphics::ShaderValueType::Bool:
        return MaterialValueType::Bool;
    case Graphics::ShaderValueType::Int:
        return MaterialValueType::Int;
    case Graphics::ShaderValueType::Float:
        return MaterialValueType::Float;
    case Graphics::ShaderValueType::Float2:
        return MaterialValueType::Float2;
    case Graphics::ShaderValueType::Float3:
        return MaterialValueType::Float3;
    case Graphics::ShaderValueType::Float4:
        return MaterialValueType::Float4;
    }
    return MaterialValueType::Float;
}

bool fail(Error &error, const std::string &message)
{
    error.message = message;
    return false;
}

MaterialValue valueFromDefaults(MaterialValueType type, const std::vector<float> &numbers)
{
    if (numbers.empty()) {
        return DefaultValueFor(type);
    }
    switch (type) {
    case MaterialValueType::Bool:
        return numbers[0] != 0.0f;
    case MaterialValueType::Int:
        return static_cast<std::int32_t>(numbers[0]);
    case MaterialValueType::Float:
        return numbers[0];
    case MaterialValueType::Float2:
        return Maths::Vector2{numbers[0], numbers[1]};
    case MaterialValueType::Float3:
        return Maths::Vector3{numbers[0], numbers[1], numbers[2]};
    case MaterialValueType::Float4:
        return Maths::Vector4{numbers[0], numbers[1], numbers[2], numbers[3]};
    case MaterialValueType::Texture:
        break;
    }
    return DefaultValueFor(type);
}

bool parseDefaultTexture(const std::string &name, DefaultTexture &texture)
{
    if (name == "white") {
        texture = DefaultTexture::White;
    } else if (name == "black") {
        texture = DefaultTexture::Black;
    } else if (name == "normal") {
        texture = DefaultTexture::FlatNormal;
    } else {
        return false;
    }
    return true;
}

} // namespace

MaterialValue DefaultValueFor(MaterialValueType type)
{
    switch (type) {
    case MaterialValueType::Bool:
        return false;
    case MaterialValueType::Int:
        return std::int32_t{0};
    case MaterialValueType::Float:
        return 0.0f;
    case MaterialValueType::Float2:
        return Maths::Vector2();
    case MaterialValueType::Float3:
        return Maths::Vector3();
    case MaterialValueType::Float4:
        return Maths::Vector4();
    case MaterialValueType::Texture:
        return MaterialTexture();
    }
    return 0.0f;
}

bool MaterialLayout::Build(
    const std::vector<const Graphics::ShaderReflection *> &stages,
    std::shared_ptr<const MaterialLayout> &layout,
    Error &error)
{
    auto built = std::make_shared<MaterialLayout>();
    bool haveBuffer = false;
    std::string bufferName;

    for (const Graphics::ShaderReflection *stage : stages) {
        for (const Graphics::BindingDesc &binding : stage->bindings) {
            if (binding.set != kMaterialSet) {
                continue;
            }
            if (binding.type != Graphics::BindingType::UniformBuffer
                && binding.type != Graphics::BindingType::SampledTexture) {
                return fail(error, "Material resource '" + binding.name
                    + "' must be a constant buffer or a sampled texture.");
            }
            if (binding.arrayCount != 1) {
                return fail(error, "Material resource '" + binding.name
                    + "' cannot be an array.");
            }
            if (binding.type != Graphics::BindingType::SampledTexture) {
                continue;
            }
            const auto existing = built->Find(binding.name);
            if (existing) {
                const MaterialParameter &parameter = built->m_Parameters[*existing];
                if (parameter.type != MaterialValueType::Texture
                    || parameter.binding != binding.binding) {
                    return fail(error, "Shader stages declare material texture '"
                        + binding.name + "' differently.");
                }
                continue;
            }
            MaterialParameter parameter;
            parameter.name = binding.name;
            parameter.type = MaterialValueType::Texture;
            parameter.binding = binding.binding;
            parameter.defaultValue = DefaultValueFor(parameter.type);
            for (const Graphics::ShaderTextureDefault &item : stage->textureDefaults) {
                if (item.set == binding.set && item.binding == binding.binding
                    && !parseDefaultTexture(item.texture, parameter.defaultTexture)) {
                    return fail(error, "Material texture '" + binding.name
                        + "' has an unknown default '" + item.texture + "'.");
                }
            }
            built->m_Parameters.push_back(std::move(parameter));
        }

        for (const Graphics::ShaderUniformBlock &block : stage->uniformBlocks) {
            if (block.set != kMaterialSet) {
                continue;
            }
            if (haveBuffer && (block.name != bufferName || block.size != built->m_BufferSize
                    || block.binding != built->m_BufferBinding)) {
                return fail(error, "A program may declare only one material constant buffer; "
                    "found '" + block.name + "' and '" + bufferName + "'.");
            }
            if (!haveBuffer) {
                haveBuffer = true;
                bufferName = block.name;
                built->m_BufferSize = block.size;
                built->m_BufferBinding = block.binding;
            }
            for (const Graphics::ShaderBufferMember &member : block.members) {
                const MaterialValueType type = toMaterialType(member.type);
                const auto existing = built->Find(member.name);
                if (existing) {
                    const MaterialParameter &parameter = built->m_Parameters[*existing];
                    if (parameter.type != type || parameter.offset != member.offset) {
                        return fail(error, "Shader stages declare material parameter '"
                            + member.name + "' differently.");
                    }
                    continue;
                }
                MaterialParameter parameter;
                parameter.name = member.name;
                parameter.type = type;
                parameter.offset = member.offset;
                parameter.defaultValue = valueFromDefaults(type, member.defaultValue);
                built->m_Parameters.push_back(std::move(parameter));
            }
        }
    }

    if (built->m_Parameters.empty() && !haveBuffer) {
        layout = nullptr;
        return true;
    }
    layout = std::move(built);
    return true;
}

std::optional<std::size_t> MaterialLayout::Find(const std::string &name) const noexcept
{
    for (std::size_t index = 0; index < m_Parameters.size(); ++index) {
        if (m_Parameters[index].name == name) {
            return index;
        }
    }
    return std::nullopt;
}

std::vector<std::uint8_t> MaterialLayout::Pack(const std::vector<MaterialValue> &values) const
{
    std::vector<std::uint8_t> bytes(m_BufferSize, 0);
    const auto write = [&bytes](std::uint32_t offset, const void *source, std::size_t size) {
        if (offset + size <= bytes.size()) {
            std::copy_n(static_cast<const std::uint8_t *>(source), size, bytes.begin() + offset);
        }
    };
    for (std::size_t index = 0; index < m_Parameters.size() && index < values.size(); ++index) {
        const MaterialParameter &parameter = m_Parameters[index];
        const MaterialValue &value = values[index];
        switch (parameter.type) {
        case MaterialValueType::Bool: {
            const std::uint32_t flag = std::get<bool>(value) ? 1u : 0u;
            write(parameter.offset, &flag, sizeof(flag));
            break;
        }
        case MaterialValueType::Int: {
            const std::int32_t number = std::get<std::int32_t>(value);
            write(parameter.offset, &number, sizeof(number));
            break;
        }
        case MaterialValueType::Float: {
            const float number = std::get<float>(value);
            write(parameter.offset, &number, sizeof(number));
            break;
        }
        case MaterialValueType::Float2: {
            const Maths::Vector2 &vector = std::get<Maths::Vector2>(value);
            write(parameter.offset, &vector, sizeof(vector));
            break;
        }
        case MaterialValueType::Float3: {
            const Maths::Vector3 &vector = std::get<Maths::Vector3>(value);
            write(parameter.offset, &vector, sizeof(vector));
            break;
        }
        case MaterialValueType::Float4: {
            const Maths::Vector4 &vector = std::get<Maths::Vector4>(value);
            write(parameter.offset, &vector, sizeof(vector));
            break;
        }
        case MaterialValueType::Texture:
            break;
        }
    }
    return bytes;
}

} // namespace Engine
