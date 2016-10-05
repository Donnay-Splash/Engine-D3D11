#include "Globals_ps.hlsl"

float4 PSMain(PixelInputType input) : SV_TARGET
{
    float3 fragmentColor = 0.0f;
    float3 normal = normalize(input.normal);
    float intensity = 1.0f;
    float3 materialDiffuse = material_diffuseColorAndOpacity.rgb;
    float3 emissiveColor = material_emissiveColor.rgb;
    if(material_hasDiffuseTexture == true)
        materialDiffuse = DiffuseTexture.Sample(DiffuseSampler, input.tex);
    if(material_hasEmissiveTexture == true)
        emissiveColor = EmissiveTexture.Sample(EmissiveSampler, input.tex);

    [unroll]
    for (float i = 0; i < activeLights; i++)
    {
        Light light = lights[i];
        float3 lightDir = normalize(light.position);
        float attenuation = 1.0f; /// pow(length(lightDir), 2);
        float lightContribution = saturate(dot(normalize(lightDir), normal)) * attenuation;
        fragmentColor += materialDiffuse * light.color.rgb * lightContribution * intensity;
        fragmentColor += emissiveColor;
    }

    return float4(fragmentColor, 1.0f);
}


