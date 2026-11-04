#pragma once

#include <string>
#include <vector>

namespace ShaderCompiler {

struct DxcBufferMember
{
    std::string name;
    std::string type; // HLSL type as printed by dxc, e.g. "float4" or "float4x4".
    unsigned offset = 0;
};

struct DxcBufferBlock
{
    std::string name;
    unsigned size = 0;
    std::vector<DxcBufferMember> members;
};

struct DxcResourceBinding
{
    std::string name;
    std::string kind; // "cbuffer", "texture", "sampler" or "UAV".
    unsigned slot = 0;
    unsigned space = 0;
};

// Reads the "Resource Bindings" table, which carries the declared names that Metal Shader
// Converter reflection leaves empty.
std::vector<DxcResourceBinding> ParseDxcResourceBindings(const std::string &disassembly);

// Reads the "Buffer Definitions" section of dxc's disassembly (-Fc), which lists every
// constant buffer's member names, types and byte offsets.
std::vector<DxcBufferBlock> ParseDxcBufferBlocks(const std::string &disassembly);

} // namespace ShaderCompiler
