#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "Lighting.hlsl"
#include "PostEffectConstants.hlsl"

// GBuffer layout
Texture2DArray diffuseColor : register(t0);
Texture2DArray csNormals : register(t1);
Texture2DArray ssVelocity : register(t2);
Texture2DArray csZ : register(t3);
Texture2DArray depth : register(t4);
Texture2D AO : register(t5);
Texture2D radiosityTexture : register(t6);
SamplerState gBufferSampler : register(s0);


float4 PSMain(VertexOut input) : SV_Target
{
    float4 color = 0.0f.xxxx;
    float target = floor(gBufferTarget);
    float3 samplePoint = float3(input.uv, displaySecondLayer);
    float z = csZ.Sample(gBufferSampler, samplePoint).r;
    float3 csPosition = ReconstructCSPosition(input.position.xy, z, projectionInfo);
    float3 ao = AO.Sample(gBufferSampler, samplePoint.xy).xxx;
    
    float4 diffuseSample = diffuseColor.Sample(gBufferSampler, samplePoint);
    float3 baseColor = diffuseSample.rgb;

    float roughness = max(1.0f - diffuseSample.a * 0.7f, 0.001f);
    float alpha = roughness * roughness;

    float3 normal = normalize((csNormals.Sample(gBufferSampler, samplePoint).rgb * 2.0f) - 1.0f);

    float3 viewDirection = normalize(-csPosition);

    float4 radiositySample = radiosityTexture.Sample(gBufferSampler, input.uv);
    const float radiosityContrastCentre = 0.35f;
    float confidence = radiositySample.a;
    confidence = saturate(((1.0f - confidence) - radiosityContrastCentre) * 2.0f + radiosityContrastCentre);
    float3 radiosity = confidence * radiositySample.rgb * radiosityEnabled;


    float3 radiance = 0.0f;
    radiance += EvaluateBRDF(normal, viewDirection, -normalize(lights[0].direction), alpha, baseColor) * lights[0].color.rgb;
    radiance += EvaluateBRDF(normal, viewDirection, -normalize(lights[1].direction), alpha, baseColor) * lights[1].color.rgb;
    float3 aoContribution = lerp(1.0f.xxx, ao, aoEnabled);
    radiance += radiosity * aoContribution * baseColor;
    radiance += 0.2f * aoContribution * lerp(1.0f, 0.3f, confidence) * baseColor;

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
        float2 ssVel = ssVelocity.Sample(gBufferSampler, samplePoint).rg;
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
    else if(target == 5.0f)
    {
        color.rgb = ao;
    }
    else if(target == 6.0f)
    {
        color.rgb = radiosity;
    }

    
    return float4(color.rgb, 1.0f);
}