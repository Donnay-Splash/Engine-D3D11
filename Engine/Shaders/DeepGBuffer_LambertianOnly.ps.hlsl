#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "Lighting.hlsl"
#include "PostEffectConstants.hlsl"

// Calculates basic lambertian lighting for both layers of deep G-Buffer
// Then packs normals of both layers into a single texture.
// TODO: Rename this shader to something more appropriate. e.g. IndirectLightPrep, RadiosityPrep, RadiosityGBuffer

Texture2DArray diffuseColorTexture : register(t0);
Texture2DArray csNormalsTexture : register(t1);
Texture2DArray ssVelocityTexture : register(t2);
Texture2DArray csZTexture : register(t3);
Texture2DArray depthTexture : register(t4);
Texture2D previousRadiosityTexture : register(t5);
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
    result.firstLayer = 0.0f;
    result.secondLayer = 0.0f;

    // Shading data for each layer
    float3 diffuseColor[2];
    float3 csNormal[2];
    float csZ[2];
    float3 csPosition[2];
    float3 viewDirection[2];

    [unroll]
    for (int i = 0; i < 2; i++)
    {
        float3 samplePoint = float3(input.uv, i);
        diffuseColor[i] = diffuseColorTexture.Sample(gBufferSampler, samplePoint).rgb;
        float3 normal = csNormalsTexture.Sample(gBufferSampler, samplePoint).rgb;
        csNormal[i] = normalize((normal * 2.0f) - 1.0f);
        csZ[i] = csZTexture.Sample(gBufferSampler, samplePoint).r;
        csPosition[i] = ReconstructCSPosition(input.position.xy, csZ[i], projectionInfo);
        viewDirection[i] = normalize(-csPosition[i]);
    }

    
    // Calculate lambert lighting for all lights
    [unroll]
    for (int lightNum = 0; lightNum < activeLights; lightNum++)
    {
        Light light = lights[lightNum];
        float3 lightDirection = -normalize(light.direction);
        result.firstLayer.rgb += EvaluateLambert(csNormal[0], lightDirection, diffuseColor[0]) * light.color.rgb;
        result.secondLayer.rgb += EvaluateLambert(csNormal[1], lightDirection, diffuseColor[1]) * light.color.rgb;
    }

    // apply indirect to first layer

    // Pack normal for both channels
    // This creates some noise on bumpy normal mapped surfaces. Might need to use standard normals
    result.packedNormal.xy = PackNormal(csNormal[0]);
    result.packedNormal.zw = PackNormal(csNormal[1]);

    // Return calculated results
    return result;
}