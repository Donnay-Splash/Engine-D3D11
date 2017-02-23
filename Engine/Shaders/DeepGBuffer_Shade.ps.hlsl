#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "Lighting.hlsl"

// GBuffer layout
Texture2DArray diffuseColor : register(t0);
Texture2DArray csNormals : register(t1);
Texture2DArray ssVelocity : register(t2);
Texture2DArray csZ : register(t3);
Texture2DArray depth : register(t4);
SamplerState gBufferSampler : register(s0);

cbuffer Constants : register(b1)
{
    float displaySecondLayer;
    float gBufferTarget;
}

#define MAX_LIGHT_COUNT 4
// struct containing light information
struct Light
{
    float3 position;
    float3 direction;
    float4 color;
};

// Contains description of the lights in the scene
cbuffer LightBuffer : register(b2)
{
    Light lights[MAX_LIGHT_COUNT];
    float activeLights;
}

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 PSMain(VertexOut input) : SV_Target
{
    float4 color = 0.0f.xxxx;
    float target = floor(gBufferTarget);
    float3 samplePoint = float3(input.uv, displaySecondLayer);
    float z = csZ.Sample(gBufferSampler, samplePoint).r;
    float3 csPosition = ReconstructCSPosition(input.position.xy, z, projectionInfo);
    
    float4 diffuseSample = diffuseColor.Sample(gBufferSampler, samplePoint);
    float3 baseColor = diffuseSample.rgb;
    float roughness = max(1.0f - diffuseSample.a * 0.7f, 0.001f);
    float alpha = roughness * roughness;
    float3 normal = normalize((csNormals.Sample(gBufferSampler, samplePoint).rgb * 2.0f) - 1.0f);
    float3 viewDirection = normalize(-csPosition);
    float3 radiance = 0.0f;
    radiance += EvaluateBRDF(normal, viewDirection, -normalize(lights[0].direction), alpha, baseColor) * lights[0].color.rgb;
    radiance += EvaluateBRDF(normal, viewDirection, -normalize(lights[1].direction), alpha, baseColor) * lights[1].color.rgb;

    if(target == 0.0f)
    {
        color.rgb = saturate(radiance);
    }
    else if (target == 1.0f)
    {
        color = csNormals.Sample(gBufferSampler, samplePoint);
    }
    else if (target == 2.0f)
    {
        float2 ssVel = ssVelocity.Sample(gBufferSampler, samplePoint);
        color = float4((ssVel + 1.0f) * 0.5f, 0.5f, 1.0f);
    }
    else if(target == 3.0f)
    {
        color = 1.0f.xxxx - depth.Sample(gBufferSampler, samplePoint).xxxx;
    }
    else if(target == 4.0f)
    {
        color = csPosition.xyzz / 1000.0f;
    }

    return float4(GammaEncode(color.rgb), 1.0f);
}