float3 DiffuseEnvironementLighting(float3 wsN)
{
    if(environmentMapEnabled == 0.0f)
    {
        return 0.0f.xxx;
    }

    return environmentMap.SampleLevel(envSampler, wsN, MAX_ENV_MIP_LEVEL).rgb;
}

float3 SpecularEnvironmentLighting(float3 wsN, float3 wsV, float roughness)
{
    if (environmentMapEnabled == 0.0f)
    {
        return 0.0f.xxx;
    }

    float NoV = saturate(dot(wsN, wsV));
    float3 F = F_Schlick(float3(0.04f, 0.04f, 0.04f), float3(1.0f, 1.0f, 1.0f), NoV);

    float3 wsR = reflect(-wsV, wsN);

    roughness = clamp(roughness, 0.0f, 1.0f);
    float mipLevel = roughness * (environmentMapMips - 1.0f);

    return F * (environmentMap.SampleLevel(envSampler, wsR, mipLevel).rgb);
}