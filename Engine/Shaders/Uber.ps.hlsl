#include "Globals_ps.hlsl"

float4 PSMain(PixelInputType input) : SV_TARGET
{
    float3 fragmentColor = 0.0f;
    float3 normal = normalize(input.normal);
    float intensity = 1.0f;
    [unroll]
    for (float i = 0; i < activeLights; i++)
    {
        Light light = lights[i];
        float3 lightDir = normalize(light.position);
        float attenuation = 1.0f; /// pow(length(lightDir), 2);
        float lightContribution = saturate(dot(normalize(lightDir), normal)) * attenuation;
        fragmentColor += light.color.rgb * lightContribution * intensity;
    }

    return float4(fragmentColor, 1.0f);
}


