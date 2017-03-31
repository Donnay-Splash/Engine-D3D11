/*  Splits the scene into near and far fields depending on depth and DoF settings*/

#include "PostEffectConstants.hlsl"
#include "DoFConstants.hlsl"

// As input take camera space Z and shaded scene
Texture2D colourTexture : register(t0);
Texture2D csZTexture : register(t1);
SamplerState dofSampler : register(s0);

struct PixelOut
{
    float3 nearPlane : SV_Target0;
    float3 farPlane : SV_Target1;
    float2 CoC : SV_Target2;
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
    // TODO: When deep G-Buffer blending not enabled. store near field colour
    //       else store deep G-Buffer data

    result.nearPlane.rgb = blend * colour;
    result.CoC.r = lerp(-1.0f, radius, blend);
    result.farPlane.rgb = colour;
    result.CoC.g = radius;

    return result;
}