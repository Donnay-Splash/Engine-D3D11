
// Stores object information. e.g. object local to world transform.
cbuffer ObjectBuffer : register(b1)
{
    row_major matrix objectToWorldTransform;
    row_major matrix objectToCameraTransform;
    row_major matrix prevObjectToCameraTransform;
}

cbuffer shadowConstants : register(b6)
{
    row_major matrix worldToProjectedLightSpace; // concatenated view and projection transform for the light
}

struct VertexInput
{
    float4 position : POSITION0;
};

/*  Super basic shader for rendering shadows. Transforms vertices into light space which render to depth.
    No pixel shader is bound. */
float4 VSMain(VertexInput input) : SV_Position
{
    input.position.w = 1.0f;

    float4 outputPos = mul(input.position, objectToWorldTransform);
    outputPos = mul(outputPos, worldToProjectedLightSpace);

    return outputPos;
}