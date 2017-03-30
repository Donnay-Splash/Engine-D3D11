/*  Splits the scene into near and far fields depending on depth and DoF settings*/

#include "PostEffectConstants.hlsl"
#include "DoFConstants.hlsl"

// As input take camera space Z and shaded scene
Texture2D colourTexture : register(t0);
Texture2D csZTexture : register(t1);
SamplerState dofSampler : register(s0);

struct PixelOut
{
    float4 packedColorAndCoC : SV_Target0;
    float4 nearPlane : SV_Target1;
    float4 farPlane : SV_Target2;
};

PixelOut PSMain(VertexOut input)
{
    PixelOut result;

    float3 colour = colourTexture.Sample(dofSampler, input.uv).rgb;
    float z = csZTexture.Sample(dofSampler, input.uv).r;
    float radius;

    if(z < nearSharpPlaneZ)
    {
        // In front of focus area
        radius = (nearSharpPlaneZ - max(z, nearBlurryPlaneZ)) * nearScale; // CoC [0, 1]
    }
    else if(z < farSharpPlaneZ)
    {
        // In focus area
        radius = 0.0f;
    }
    else
    {
        // behind focus area
        radius = (farSharpPlaneZ - min(z, farBlurryPlaneZ)) * farScale; // CoC [-1, 0]
    }

    float blend = smoothstep(0.0f, 0.05f, radius);

    // TODO: Mix in second layer into far plane colour for areas of low blend

    result.packedColorAndCoC = float4(colour, radius);
    result.nearPlane.rgb = blend * colour;
    result.nearPlane.a = radius;
    result.farPlane.rgb = (1 - floor(blend)) * colour;
    result.farPlane.a = radius;
    return result;
}