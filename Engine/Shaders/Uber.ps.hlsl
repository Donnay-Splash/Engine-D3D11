#include "Globals_ps.hlsl"

// Isotropic Normal Distribution Functions (NDF) 
// n = normal
// h = half normal
// a = perceptual roughness (roughnes ^ 2)
float square(float a)
{
    return a * a;
}

float D_BlinnPhong(float NoH, float a)
{
    float a2 = square(a);
    float normalizationFactor = 1.0f / (PI * a2);
    float power = (2.0f / a2) - 2.0f;
    float D = normalizationFactor * pow(NoH, power);
    return D;
}

float D_Beckmann(float NoH, float a)
{
    float a2 = square(a);
    float NoH2 = square(NoH);
    float D = exp((NoH2 - 1.0f) / (a2 * NoH2)) / (PI * a2 * square(NoH2));
    return D;
}

float D_GGXTrowbridgeReitz(float NoH, float a)
{
    float a2 = square(a);
    float t = 1.0f + (a2 - 1.0f) * square(NoH);
    return a2 / (PI * square(t));
}

// Geometry shadowing functions
// N = normal
// H = half normal
// L = light vector
// V = view vector
float G_CookTorrance(float NoL, float NoV, float NoH, float VoH)
{
    float a = min(NoV, NoL);
    float b = (2 * NoH * a) / VoH;
    return min(1.0f, b);
}

float G_Kelemen(float NoL, float NoV, float VoH)
{
    float VoH2 = square(VoH);
    float G = (NoL * NoV) / VoH2;
    return G;
}

// Geometry shadowing smith functions
float G_Smith_ShlickBeckman(float NoV, float NoL, float a)
{
    float k = a * sqrt(2.0f / PI);
    float GNoV = NoV / (NoV * (1.0f - k) + k);
    float GNoL = NoL / (NoL * (1.0f - k) + k);
    
    return GNoL * GNoV / (4.0f * NoV * NoL);
}

float G_Smith_GGX(float NoV, float NoL, float a)
{
    // Optimised version taken from http://graphicrants.blogspot.co.uk/2013/08/specular-brdf-reference.html
    float a2 = square(a);
    float GGXV = NoV + sqrt( (NoV - NoV * a2) * NoV + a2);
    float GGXL = NoL + sqrt( (NoL - NoL * a2) * NoL + a2);

    return rcp(GGXV * GGXL);
}

// Fresnel approximations
// F0 = Reflectance at normal incidence
// F90 = Reflectance at grazing angle. Usually one or the specular color
// HoL = dot product between the half normal vector and the
//       light direction.
float3 F_Schlick(float3 F0, float3 F90, float HoL)
{
    return F0 + (F90 - F0) * pow(1.0f - HoL, 5.0f);
}

// Spherical Gaussian approximation used in UE4
// https://seblagarde.wordpress.com/2012/06/03/spherical-gaussien-approximation-for-blinn-phong-phong-and-fresnel/
float F_SchlickGaussian(float F0, float F90, float HoL)
{
    return F0 + (F90 - F0) * exp2((-5.55473f * HoL - 6.98316f) * HoL);
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
    float3 fragmentColor = 0.0f;
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.cameraPosition - input.worldSpacePosition.xyz);
    float3 halfVector = normalize(normal + viewDir);

    float NoV = abs(dot(normal, viewDir));
    float NoH = saturate(dot(halfVector, normal));
    float HoV = saturate(dot(halfVector, viewDir));

    float intensity = 1.0f;
    float3 materialDiffuse = material_diffuseColorAndOpacity.rgb;
    float3 emissiveColor = material_emissiveColor.rgb;
    float3 specularColor = material_specularColorAndSmoothness.rgb;
    // Rename these. 1 - smoothness is linear roughness.
    // linear roughness squared is the GGX alpha term.
    float roughness = saturate(1.0f - material_specularColorAndSmoothness.a);
    float perceptualRoughness = pow(roughness, 2);
    float D = D_GGXTrowbridgeReitz(NoH, perceptualRoughness);

    if(material_hasDiffuseTexture == true)
        materialDiffuse = DiffuseTexture.Sample(DiffuseSampler, input.tex).rgb;
    if(material_hasEmissiveTexture == true)
        emissiveColor = EmissiveTexture.Sample(EmissiveSampler, input.tex).rgb;

    
    float3 lightDir = -normalize(float3(0.0f, 0.0f, -1.0f));
    float NoL = saturate(dot(normal, lightDir));
    float HoL = saturate(dot(halfVector, lightDir));
    float G = G_Smith_GGX(NoV, NoL, perceptualRoughness);
    float3 F = F_Schlick(float3(0.14f, 0.14f, 0.14f), float3(1.0f, 1.0f, 1.0f), HoL);
    float attenuation = 1.0f; /// pow(length(lightDir), 2);
    float3 RDiffuse = (materialDiffuse * NoL) / PI;
    float3 RSpec = D * G * F;
    fragmentColor += (RDiffuse + RSpec);

    return float4(fragmentColor, 1.0f);
}


