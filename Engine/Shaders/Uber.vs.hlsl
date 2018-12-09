#include "DeepGBuffer_Gen_Globals.hlsl"
#include "DeepGBuffer_Gen_InputStructs.hlsl"
#include "InputStructs.hlsl"

PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output;
    input.position.w = 1.0f;
    // Store the texture coordinates for the pixel shader.
    output.position = mul(input.position, objectToCameraTransform);
    output.worldSpacePosition = output.position;
    output.position = mul(output.position, jitteredProjectionMatrix);

    // Since we don't allow non uniform scaling we are safe to use the modelView matrix
    // instead of having to use the transposed inverse of the modelView matrix.
    output.normal = mul(float4(input.normal, 0.0f), objectToCameraTransform).xyz;
    output.tex = input.tex;
    output.cameraPosition = cameraPos;
    return output;
}