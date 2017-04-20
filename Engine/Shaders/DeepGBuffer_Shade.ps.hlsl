#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "Lighting.hlsl"
#include "PostEffectConstants.hlsl"
#include "GIHelpers.hlsl"

// GBuffer layout
Texture2DArray diffuseColor : register(t0);
Texture2DArray emissiveTexture : register(t1);
Texture2DArray csNormals : register(t2);
Texture2DArray ssVelocity : register(t3);
Texture2DArray csZ : register(t4);
Texture2DArray depth : register(t5);
SamplerState gBufferSampler : register(s0);


float3 PSMain(VertexOut input) : SV_Target0
{
    float target = floor(gBufferTarget);

    // Scene data for both layers
    float2 ssPos = (input.uv - 0.5f) * float2(2.0f, -2.0f);

    float3 samplePoint = float3(input.uv, 0);

    float z = csZ.Sample(gBufferSampler, samplePoint).r;
    float3 csPositionJit = ReconstructCSPosition(input.position.xy, z, projectionInfo);

    float3 emissiveColor = emissiveTexture.Sample(gBufferSampler, samplePoint).rgb;

    float4 diffuseSample = diffuseColor.Sample(gBufferSampler, samplePoint);
    float3 baseColor = diffuseSample.rgb;
    float roughness = max(1.0f - diffuseSample.a * 0.7f, 0.001f);
    float alpha = roughness * roughness;

    float3 normal = normalize((csNormals.Sample(gBufferSampler, samplePoint).rgb * 2.0f) - 1.0f);

    float d = depth.Sample(gBufferSampler, samplePoint).r;
    float4 csPositionNoJit = mul(float4(ssPos, d, 1.0f), invJitteredProjection);
    csPositionNoJit /= csPositionNoJit.w;


    // If z is below zero then we are drawing an empty pixel
    if (csPositionJit.z <= 0.0f)
    {
        return 0.0f.xxx;
    }

    float3 viewDirection = normalize(-csPositionJit);

    // Shadow data for both layers
    float visibility[MAX_LIGHT_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f};
    visibility[0] = SampleShadowMapPCF(csPositionNoJit.xyz);

    // Environment lighting data for both layers
    float3 wsV = normalize(mul(float4(viewDirection, 0.0f), invViewMatrix).xyz);

    float3 wsN = normalize(mul(float4(normal, 0.0f), invViewMatrix).xyz);
    float3 diffuseAmbient = DiffuseEnvironementLighting(wsN);
    float3 specularAmbient = SpecularEnvironmentLighting(wsN, wsV, alpha);

    float3 radiance = emissiveColor;
    for (int lightNum = 0; lightNum < activeLights; lightNum++)
    {
        Light light = lights[lightNum];
        float3 lightDir = -normalize(light.direction);
        radiance += EvaluateBRDF(normal, viewDirection, lightDir, alpha, baseColor) * light.color.rgb * visibility[lightNum];
    }


    radiance += diffuseAmbient * (baseColor / PI);
    radiance += specularAmbient / PI;

    return radiance;
}