#include "Globals_vs.hlsl"

PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output;
    input.position.w = 1.0f;
    // Store the texture coordinates for the pixel shader.
    output.position = mul(input.position, objectTransform);
    output.worldPosition = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = input.tex;
    output.normal = input.normal;

    return output;
}