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

// Backend-neutral description of what a single shader entry point consumes. It is written
// by the shader compiler as <Source>_<entry>.reflection.json, one schema for every backend:
//   {"stage": "vertex|fragment|compute",
//    "vertexInputs": [{"semantic", "format": "float2|float3|float4|uint", "location"}],
//    "bindings": [{"binding", "type": "uniformBuffer|storageBuffer|sampledTexture|
//                  storageTexture|sampler", "arrayCount", "name"}]}
struct ShaderReflection
{
    ShaderStage stage = ShaderStage::None;
    std::vector<ShaderVertexInput> vertexInputs;
    // Each binding's visibility is the shader's own stage.
    std::vector<BindingDesc> bindings;
};

bool ParseShaderReflection(std::string_view json, ShaderReflection &reflection, Error &error);

} // namespace Graphics
} // namespace Engine
