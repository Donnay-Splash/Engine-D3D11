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
//Texture2D AO : register(t6);
//Texture2D radiosityTexture : register(t7);
SamplerState gBufferSampler : register(s0);


struct PixelOut
{
    float3 frontLayer : SV_Target0;
    float3 backLayer : SV_Target1;
};

PixelOut PSMain(VertexOut input)
{
    PixelOut result;
    result.frontLayer = 0.0f;
    result.backLayer = 0.0f;
    float target = floor(gBufferTarget);

    float3 color[2];

    // Scene data for both layers
    float3 csPositionJit[2];
    float4 csPositionNoJit[2];
    float3 baseColor[2];
    float3 emissiveColor[2];
    float3 normal[2];
    float alpha[2];
    float2 ssPos = (input.uv - 0.5f) * float2(2.0f, -2.0f);

    // Collect data for both layers of the deep G-Buffer
    for (int i = 0; i < 2; i++)
    {
        float3 samplePoint = float3(input.uv, i);
        color[i] = 0.0f.xxxx;

        float z = csZ.Sample(gBufferSampler, samplePoint).r;
        csPositionJit[i] = ReconstructCSPosition(input.position.xy, z, projectionInfo);

        emissiveColor[i] = emissiveTexture.Sample(gBufferSampler, samplePoint).rgb;

        float4 diffuseSample = diffuseColor.Sample(gBufferSampler, samplePoint);
        baseColor[i] = diffuseSample.rgb;
        float roughness = max(1.0f - diffuseSample.a * 0.7f, 0.001f);
        alpha[i] = roughness * roughness;

        normal[i] = normalize((csNormals.Sample(gBufferSampler, samplePoint).rgb * 2.0f) - 1.0f);

        float d = depth.Sample(gBufferSampler, samplePoint).r;
        csPositionNoJit[i] = mul(float4(ssPos, d, 1.0f), invJitteredProjection);
        csPositionNoJit[i] /= csPositionNoJit[i].w;
    }


    // If z is below zero then we are drawing an empty pixel
    if (csPositionJit[0].z <= 0.0f)
    {
        return result;
    }

    float3 viewDirection = normalize(-csPositionJit[0]);
    //float3 ao = AO.Sample(gBufferSampler, samplePoint.xy).xxx;



    //float4 radiositySample = radiosityTexture.Sample(gBufferSampler, input.uv);
    //float confidence = radiositySample.a;
    //confidence = saturate(((1.0f - confidence) - radiosityContrastCentre) * 2.0f + radiosityContrastCentre);
    //float3 radiosity = confidence * radiositySample.rgb * radiosityEnabled * ColorBoost(radiositySample.rgb, unsaturatedBoost, saturatedBoost);

    // Shadow data for both layers
    float2 visibility[MAX_LIGHT_COUNT] = { 1.0f.xx, 1.0f.xx, 1.0f.xx, 1.0f.xx };
    visibility[0].r = SampleShadowMapPCF(csPositionNoJit[0].xyz);
    visibility[0].g = SampleShadowMapPCF(csPositionNoJit[1].xyz);

    // Environment lighting data for both layers
    float3 diffuseAmbient[2];
    float3 specularAmbient[2];
    float3 wsV = normalize(mul(float4(viewDirection, 0.0f), invViewMatrix).xyz);

    for (int layerNum = 0; layerNum < 2; layerNum++)
    {
        float3 wsN = normalize(mul(float4(normal[layerNum], 0.0f), invViewMatrix).xyz);
        diffuseAmbient[layerNum] = DiffuseEnvironementLighting(wsN) * envIntensity;
        specularAmbient[layerNum] = SpecularEnvironmentLighting(wsN, wsV, alpha[layerNum]) * envIntensity;
    }

    float3 radiance[2] = { emissiveColor[0], emissiveColor[1] };
    for (int lightNum = 0; lightNum < activeLights; lightNum++)
    {
        Light light = lights[lightNum];
        float3 lightDir = -normalize(light.direction);
        radiance[0] += EvaluateBRDF(normal[0], viewDirection, lightDir, alpha[0], baseColor[0]) * light.color.rgb * visibility[lightNum].r;
        radiance[1] += EvaluateBRDF(normal[1], viewDirection, lightDir, alpha[1], baseColor[1]) * light.color.rgb * visibility[lightNum].g;
    }


    //float3 aoContribution = lerp(1.0f.xxx, ao, aoEnabled);
    radiance[0] += (/*(radiosity * aoContribution)*/ +(diffuseAmbient[0] /** aoContribution * lerp(1.0f, 0.3f, confidence)*/)) * (baseColor[0] / PI);
    radiance[0] += (specularAmbient[0] /** aoContribution*/) / PI;

    radiance[1] += ( /*(radiosity * aoContribution)*/+(diffuseAmbient[1] /** aoContribution * lerp(1.0f, 0.3f, confidence)*/)) * (baseColor[1] / PI);
    radiance[1] += (specularAmbient[1] /** aoContribution*/) / PI;

    //if(target == 0.0f)
    //{
    //    color.rgb = radiance;
    //}
    //else if (target == 1.0f)
    //{
    //    color = csNormals.Sample(gBufferSampler, samplePoint);
    //}
    //else if (target == 2.0f)
    //{
    //    float2 ssVel = ssVelocity.Sample(gBufferSampler, samplePoint).rg;
    //    color = float4((ssVel + 1.0f) * 0.5f, 0.5f, 1.0f);
    //}
    //else if(target == 3.0f)
    //{
    //    color = 1.0f.xxxx - depth.Sample(gBufferSampler, samplePoint).xxxx;
    //}
    //else if(target == 4.0f)
    //{
    //    color = csPosition.xyzz / 1000.0f;
    //}
    //else if(target == 5.0f)
    //{
    //    //color.rgb = ao;
    //}
    //else if(target == 6.0f)
    //{
    //    //color.rgb = radiosity;
    //}

    color[0] = radiance[0];
    color[1] = radiance[1];
    result.frontLayer = lerp(color[0], color[1], displaySecondLayer);
    result.backLayer = lerp(color[1], color[0], displaySecondLayer);

    return result;
}