#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace ShaderCompiler {

// A `// @default(a, b, ...)` comment on the HLSL declaration of a material parameter.
struct DefaultAnnotation
{
    std::string name;
    std::vector<std::string> arguments;
    int line = 0;
};

// Finds every @default comment in an HLSL source. The name is the identifier declared on
// the same line (`float4 tint; // @default(1, 0, 0, 1)`). Malformed annotations are errors.
bool ParseDefaultAnnotations(const std::string &source, std::vector<DefaultAnnotation> &annotations,
                             std::string &error);

// Checks the arguments against a material parameter type (bool, int, float, float2..float4)
// and returns them as a JSON array of numbers (bool as 0 or 1).
bool BuildValueDefault(const DefaultAnnotation &annotation, const std::string &type,
                       nlohmann::json &value, std::string &error);

// Checks a texture default (white, black or normal) and returns it as a JSON string.
bool BuildTextureDefault(const DefaultAnnotation &annotation, nlohmann::json &value,
                         std::string &error);

} // namespace ShaderCompiler
