#pragma once

#include "DefaultAnnotations.h"

#include <string>
#include <vector>

namespace ShaderCompiler {

// Converts Metal Shader Converter reflection JSON into the engine's backend-neutral schema
// (see engine/Graphics/ShaderReflection.h). `stage` is vs, ps or cs. `dxcDisassembly` is the
// dxc -Fc output, the source of constant buffer member layouts. `defaults` are the @default
// annotations of the HLSL source, applied to material (set 1 and up) parameters by name.
bool BuildPortableReflection(const std::string &metalReflectionJson, const std::string &stage,
                             const std::string &dxcDisassembly,
                             const std::vector<DefaultAnnotation> &defaults,
                             std::string &portableJson, std::string &error);

} // namespace ShaderCompiler
