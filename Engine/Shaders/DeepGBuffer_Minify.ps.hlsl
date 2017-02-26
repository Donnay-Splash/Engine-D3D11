#include "PostEffectConstants.hlsl"

Texture2D csZ : register(t0);
SamplerState csZSampler : register(s0);

float2 PSMain(VertexOut input) : SV_Target
{
    return csZ.Sample(csZSampler, input.uv).rg;
}