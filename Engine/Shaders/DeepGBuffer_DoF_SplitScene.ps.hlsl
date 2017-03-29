/*  Splits the scene into near and far fields depending on depth and DoF settings*/

#include "PostEffectConstants.hlsl"

// As input take camera space Z and shaded scene
Texture2D colourTexture : register(t0);
Texture2D csZTexture : register(t1);
SamplerState dofSampler : register(s0);

cbuffer DoFConstants : register(b7)
{
    float dofEnabled;
    float focusPlaneZ ;
    float scale;
    float maxCoCNear;
    float maxCoCFar;
}

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
    float radius = (z - focusPlaneZ) * scale;

    radius = radius * 0.5f + 0.5f;
    float blend = smoothstep(0, 0.25, radius);

    result.packedColorAndCoC = float4(colour, radius);
    result.nearPlane.rgb = (1 - blend) * colour;
    result.farPlane.rgb = blend * colour;
    return result;
}