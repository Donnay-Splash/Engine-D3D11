#include "DefaultAnnotations.h"

#include <cstdlib>
#include <iostream>

namespace {

void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

using namespace ShaderCompiler;

} // namespace

int main()
{
    std::vector<DefaultAnnotation> annotations;
    std::string error;
    const std::string source =
        "cbuffer Params : register(b0, space1) {\n"
        "    bool flag; // @default(true)\n"
        "    float4 tint; // a note @default( 1, 0.5 ,0,1 )\n"
        "    float plain;\n"
        "};\n"
        "Texture2D baseTexture : register(t0, space1); // @default(black)\n";
    check(ParseDefaultAnnotations(source, annotations, error), "valid annotations should parse");
    check(annotations.size() == 3 && annotations[0].name == "flag"
              && annotations[1].name == "tint" && annotations[1].arguments.size() == 4
              && annotations[1].arguments[1] == "0.5" && annotations[2].name == "baseTexture",
        "annotations should capture the declared name and arguments");

    nlohmann::json value;
    check(BuildValueDefault(annotations[0], "bool", value, error) && value[0] == 1.0,
        "bool defaults should convert");
    check(BuildValueDefault(annotations[1], "float4", value, error) && value[1] == 0.5,
        "vector defaults should convert");
    check(!BuildValueDefault(annotations[1], "float3", value, error),
        "the wrong arity should be rejected");
    DefaultAnnotation fraction{"count", {"1.5"}, 1};
    check(!BuildValueDefault(fraction, "int", value, error), "int defaults must be whole");
    DefaultAnnotation text{"x", {"abc"}, 1};
    check(!BuildValueDefault(text, "float", value, error), "non-numeric defaults are rejected");
    check(BuildTextureDefault(annotations[2], value, error) && value == "black",
        "texture defaults should convert");
    DefaultAnnotation badTexture{"t", {"red"}, 1};
    check(!BuildTextureDefault(badTexture, value, error), "unknown texture defaults are rejected");

    check(!ParseDefaultAnnotations("// @default(1)\n", annotations, error),
        "an annotation without a declaration is an error");
    check(!ParseDefaultAnnotations("float x; // @default(1\n", annotations, error),
        "an unterminated annotation is an error");
    check(!ParseDefaultAnnotations("float x; // @default()\n", annotations, error),
        "an empty annotation is an error");
    return 0;
}
