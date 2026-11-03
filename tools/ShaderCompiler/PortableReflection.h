#pragma once

#include <string>

namespace ShaderCompiler {

// Converts Metal Shader Converter reflection JSON into the engine's backend-neutral schema
// (see engine/Graphics/ShaderReflection.h). `stage` is vs, ps or cs.
bool BuildPortableReflection(const std::string &metalReflectionJson, const std::string &stage,
                             std::string &portableJson, std::string &error);

} // namespace ShaderCompiler
