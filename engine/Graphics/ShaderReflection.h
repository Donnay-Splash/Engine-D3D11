#pragma once

#include "Graphics/GraphicsTypes.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Engine {
struct Error;
namespace Graphics {

struct ShaderVertexInput
{
    // Upper-case HLSL semantic including its index, e.g. "POSITION0" or "TEXCOORD0".
    std::string semantic;
    VertexFormat format = VertexFormat::Float3;
    std::uint32_t location = 0;
};

enum class ShaderValueType
{
    Bool,
    Int,
    Float,
    Float2,
    Float3,
    Float4,
};

// One scalar or vector member of a constant buffer, at the byte offset the shader expects.
struct ShaderBufferMember
{
    std::string name;
    ShaderValueType type = ShaderValueType::Float;
    std::uint32_t offset = 0;
    std::uint32_t size = 0;
};

struct ShaderUniformBlock
{
    std::string name;
    std::uint32_t set = 0;
    std::uint32_t binding = 0;
    std::uint32_t size = 0;
    std::vector<ShaderBufferMember> members;
};

// Backend-neutral description of what a single shader entry point consumes. It is written
// by the shader compiler as <Source>_<entry>.reflection.json, one schema for every backend:
//   {"stage": "vertex|fragment|compute",
//    "vertexInputs": [{"semantic", "format": "float2|float3|float4|uint", "location"}],
//    "bindings": [{"binding", "set" (optional, default 0), "type": "uniformBuffer|storageBuffer|
//                  sampledTexture|storageTexture|sampler", "arrayCount", "name",
//                  "size" and "members": [{"name", "type": "bool|int|float|float2|float3|float4",
//                  "offset", "size"}] for uniform buffers}]}
struct ShaderReflection
{
    ShaderStage stage = ShaderStage::None;
    std::vector<ShaderVertexInput> vertexInputs;
    // Each binding's visibility is the shader's own stage.
    std::vector<BindingDesc> bindings;
    // Member layouts of the uniform buffers that declare one.
    std::vector<ShaderUniformBlock> uniformBlocks;
};

bool ParseShaderReflection(std::string_view json, ShaderReflection &reflection, Error &error);

} // namespace Graphics
} // namespace Engine
