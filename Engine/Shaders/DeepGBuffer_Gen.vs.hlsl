#include "DeepGBuffer_Gen_Globals.hlsl"
#include "DeepGBuffer_Gen_InputStructs.hlsl"

GeometryInput VSMain(VertexInput input)
{
    input.position.w = 1.0f;

    GeometryInput output;
    output.position = mul(input.position, objectToCameraTransform);
    // Store the current camera space position
    output.csPosition = output.position;
    output.position = mul(output.position, projectionMatrix);

    // Calculate previous camera space position
    output.csPrevPosition = mul(input.position, prevObjectToCameraTransform);

    // Calculate camera-space normal
    // Since we don't allow non uniform scaling we are safe to use the modelView matrix
    // instead of having to use the transposed inverse of the modelView matrix.
    output.csNormal = mul(float4(input.normal, 0.0f), objectToCameraTransform).xyz;
    output.texCoord = input.tex;

    return output;
}

