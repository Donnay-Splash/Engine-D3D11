#include "PostEffectConstants.hlsl"

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    output.position = input.position;
    output.uv = input.uv;

    return output;
}