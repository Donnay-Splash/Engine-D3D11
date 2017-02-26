#include "PostEffectConstants.hlsl"

Texture2DArray DeepGBuffer_csZ : register(t0);
SamplerState csZ_Sampler : register(s0);

float2 PSMain(VertexOut input) : SV_Target
{
    float cs1 = DeepGBuffer_csZ.Sample(csZ_Sampler, float3(input.uv, 0.0f)).r;
    float cs2 = DeepGBuffer_csZ.Sample(csZ_Sampler, float3(input.uv, 1.0f)).r;

    return float2(cs1, cs2);
}
