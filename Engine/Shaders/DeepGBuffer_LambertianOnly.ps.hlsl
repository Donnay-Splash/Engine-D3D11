#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "Lighting.hlsl"
#include "PostEffectConstants.hlsl"
#include "GIHelpers.hlsl"

// Calculates basic lambertian lighting for both layers of deep G-Buffer
// Then packs normals of both layers into a single texture.
// TODO: Rename this shader to something more appropriate. e.g. IndirectLightPrep, RadiosityPrep, RadiosityGBuffer

Texture2DArray diffuseColorTexture : register(t0);
Texture2DArray emissiveTexture : register(t1);
Texture2DArray csNormalsTexture : register(t2);
Texture2DArray ssVelocityTexture : register(t3);
Texture2DArray csZTexture : register(t4);
Texture2DArray depthTexture : register(t5);
Texture2D previousRadiosityTexture : register(t6);
Texture2DArray previoudDepthTexture : register(t7);
SamplerState gBufferSampler : register(s0);

// We can output the result for both layers at the same time as we have the result
struct PixelOut
{
    float4 firstLayer: SV_Target0;
    float4 secondLayer : SV_Target1;
    float4 packedNormal : SV_Target2;
};

PixelOut PSMain(VertexOut input)
{
    PixelOut result;
    result.firstLayer = emissiveTexture.Sample(gBufferSampler, float3(input.uv, 0));
    result.secondLayer = emissiveTexture.Sample(gBufferSampler, float3(input.uv, 1));
    result.packedNormal = 0.0f;

    // Shading data for each layer
    float3 diffuseColor[2];
    float3 csNormal[2];
    float depth[2];
    float4 csPosition[2];
    float2 lightVisibility[2];

    float2 ssPos = (input.uv - 0.5f) * float2(2.0f, -2.0f);

    [unroll]
    for (int i = 0; i < 2; i++)
    {
        float3 samplePoint = float3(input.uv, i);
        diffuseColor[i] = diffuseColorTexture.Sample(gBufferSampler, samplePoint).rgb;
        float3 normal = csNormalsTexture.Sample(gBufferSampler, samplePoint).rgb;
        csNormal[i] = normalize((normal * 2.0f) - 1.0f);
        depth[i] = depthTexture.Sample(gBufferSampler, samplePoint).r;
        csPosition[i] = mul(float4(ssPos, depth[i], 1.0f), invJitteredProjection);
        csPosition[i] /= csPosition[i].w;
    }

    if (depth[0] >= 1.0f)
    {
        return result;
    }

    lightVisibility[0].r = SampleShadowMapRaw(csPosition[0].xyz);
    lightVisibility[0].g = SampleShadowMapRaw(csPosition[1].xyz);
    lightVisibility[1] = 1.0f;
    
    // Calculate lambert lighting for all lights
    [unroll]
    for (int lightNum = 0; lightNum < activeLights; lightNum++)
    {
        Light light = lights[lightNum];
        float3 lightDirection = -normalize(light.direction);
        result.firstLayer.rgb += EvaluateLambert(csNormal[0], lightDirection, diffuseColor[0]) * light.color.rgb * lightVisibility[lightNum].r;
        result.secondLayer.rgb += EvaluateLambert(csNormal[1], lightDirection, diffuseColor[1]) * light.color.rgb * lightVisibility[lightNum].g;
    }

    // apply indirect to first layer
    float2 ssV = ssVelocityTexture.Sample(gBufferSampler, float3(input.uv, 0.0f)).rg;
    float2 previousCoord = input.uv - (0.5f * ssV);
    bool offscreen = max(previousCoord.x, previousCoord.y) >= 1.0f || min(previousCoord.x, previousCoord.y) <= 0.0f;

    float4 previousRadiosity = previousRadiosityTexture.Sample(gBufferSampler, previousCoord);
    float3 indirect = previousRadiosity.rgb / PI;
    float previousDepth = previoudDepthTexture.Sample(gBufferSampler, float3(previousCoord, 0.0f)).r;
    float3 previousWSPosition = ReconstructWSPosition(previousCoord / invViewSize, ReconstructCSZ(previousDepth, clipInfo), projectionInfo, prevInvViewMatrix);

    float3 wsPosition = mul(float4(csPosition[0].xyz, 1.0f), invViewMatrix).xyz;

    float dist = length(wsPosition - previousWSPosition);
    float weight = 1.0f - smoothstep(0.5f, 0.7f, dist) * float(!offscreen);

    indirect *= weight;
    result.firstLayer.rgb += indirect * (1.0f - radiosityPropogationDamping) * ColorBoost(indirect, unsaturatedBoost, saturatedBoost) * diffuseColor[0];
    // Pack normal for both channels
    // This creates some noise on bumpy normal mapped surfaces. Might need to use standard normals
    result.packedNormal.xy = PackNormal(csNormal[0]);
    result.packedNormal.zw = PackNormal(csNormal[1]);

    // Return calculated results
    return result;
}