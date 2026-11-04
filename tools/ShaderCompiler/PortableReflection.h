#pragma once

#include <string>

namespace ShaderCompiler {

// Converts Metal Shader Converter reflection JSON into the engine's backend-neutral schema
// (see engine/Graphics/ShaderReflection.h). `stage` is vs, ps or cs. `dxcDisassembly` is the
// dxc -Fc output, the source of constant buffer member layouts.
bool BuildPortableReflection(const std::string &metalReflectionJson, const std::string &stage,
                             const std::string &dxcDisassembly, std::string &portableJson,
                             std::string &error);

} // namespace ShaderCompiler
