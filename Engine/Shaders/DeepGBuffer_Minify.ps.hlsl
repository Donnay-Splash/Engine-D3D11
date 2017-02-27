#include "PostEffectConstants.hlsl"

Texture2D csZ : register(t0);
SamplerState csZSampler : register(s0);

float2 PSMain(VertexOut input) : SV_Target
{
    int2 fragCoord = int2(input.position.xy);
    uint width, height, levels;
    csZ.GetDimensions(uint(0), width, height, levels);
    int2 samplePoint = clamp(2 * fragCoord + int2(fragCoord.y & 1, fragCoord.x & 1), 0, int2(width, height) - int2(1, 1));
    return csZ.Load(int3(samplePoint, 0)).rg;
}