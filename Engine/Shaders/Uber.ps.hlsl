#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "Lighting.hlsl"
#include "GIHelpers.hlsl"
#include "InputStructs.hlsl"

// Textures and samplers
Texture2D DiffuseTexture : register(t0);
SamplerState MaterialSampler : register(s0);

Texture2D SpecularTexture : register(t1);

Texture2D EmissiveTexture : register(t2);

Texture2D NormalTexture : register(t3);

Texture2D SmoothnessTexture : register(t4);

Texture2D OpacityTexture : register(t5);

Texture2D AOTexture : register(t6);

cbuffer MaterialBuffer : register(b3)
{
	float4 material_diffuseColorAndOpacity;
	float4 material_specularColorAndSmoothness;
	float4 material_emissiveColor;
	float material_emissiveIntensity;
	bool material_hasDiffuseTexture;
	bool material_hasSpecularTexture;
	bool material_hasEmissiveTexture;
	bool material_hasNormalTexture;
	bool material_hasSmoothnessTexture;
	bool material_hasOpacityTexture;
	bool material_hasAOTexture;
}

float4 PSMain(VertexOut input) : SV_Target
{
    // Get normalised vectors
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.cameraPosition - input.worldSpacePosition.xyz);

    float NoV = abs(dot(normal, viewDir)) + 1e-5f; // Avoid artifact
    
    float intensity = 1.0f;
    float3 materialDiffuse = material_diffuseColorAndOpacity.rgb;
    float3 emissiveColor = material_emissiveColor.rgb;
    float3 specularColor = material_specularColorAndSmoothness.rgb;
    float roughness = 1.0f - material_specularColorAndSmoothness.a;
    float alpha = max(0.001, square(roughness));

    // Since we are using SRGB formats for the textures the conversion to linear is done when reading
    if(material_hasDiffuseTexture == true)
        materialDiffuse = DiffuseTexture.Sample(MaterialSampler, input.tex).rgb;
    if(material_hasEmissiveTexture == true)
        emissiveColor = EmissiveTexture.Sample(MaterialSampler, input.tex).rgb;

    float3 radiance = 0.0f;
    [unroll]
    for (int i = 0; i < activeLights; i++)
    {
        Light light = lights[i];
        float3 lightDir = -normalize(light.position);
      
        radiance += EvaluateBRDF(normal, viewDir, lightDir, alpha, materialDiffuse) * light.color.rgb;
    }

    // Gamma encode
    return float4(GammaEncode(saturate(radiance)), 1.0f);
}


