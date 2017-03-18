#include "PostEffectConstants.hlsl"
#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"

Texture2D currentFrame : register(t0);
Texture2D previousFrame : register(t1);
Texture2D currentFrameCsZ : register(t2);
Texture2DArray previousDepth : register(t3);
Texture2DArray ssVelocity : register(t4);
SamplerState bufferSampler : register(s0);

float4 PSMain(VertexOut input) : SV_Target
{
    float2 ssPosition = input.position.xy;
    float csZ = currentFrameCsZ.Sample(bufferSampler, input.uv).r;
    float3 wsPosition = ReconstructWSPosition(ssPosition, csZ, projectionInfo, invViewMatrix);

    float4 currentValue = currentFrame.Sample(bufferSampler, input.uv);
    float2 ssVel = ssVelocity.Sample(bufferSampler, float3(input.uv, 0.0f)).rg;

    float2 prevSamplePoint = input.uv - (0.5f *ssVel);
    float4 previousValue = previousFrame.Sample(bufferSampler, prevSamplePoint);

    float previousZ = previousDepth.Sample(bufferSampler, float3(prevSamplePoint, 0.0f)).r;
    float prevcsZ = ReconstructCSZ(previousZ, clipInfo);
    float3 prevWSPosition = ReconstructWSPosition(prevSamplePoint /invViewSize, prevcsZ, projectionInfo, prevInvViewMatrix);

    float dist = length(wsPosition - prevWSPosition);

    float weight = basicTemporalFilterBlend * (1.0f - smoothstep(0.5f, 0.7f, dist));

    return lerp(currentValue, previousValue, weight);
}