#include "Globals_ps.hlsl"

// Isotropic Normal Distribution Functions (NDF) 
// n = normal
// h = half normal
// a = perceptual roughness (roughnes ^ 2)
float square(float a)
{
    return a * a;
}

float3 GammaEncode(float3 linearColor)
{
    return pow(linearColor, kGammaEncodePower);
}

float3 GammaDecode(float3 gammaColor)
{
    return pow(gammaColor, kGammaDecodePower);
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
    // Note: At roughness of 0 t can be 0
    // Thus resulting in a divide by zero at the final step.
    float t = 1.0f + (a2 - 1.0f) * square(NoH);
    return a2 / (PI * square(t) + 1e-5f);
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

float3 EvaluateBRDF(float3 normal, float3 viewDirection, float3 lightDirection, float roughness, float3 baseColor)
{
    float NoL = dot(normal, lightDirection);
    float NoV = abs(dot(normal, viewDirection)) + 1e-5f; // Avoid artifacts
    // No contribution if the fragment isn't visible
    // from the camera or from the light
    if (NoV < 0.0f || NoL < 0.0f)
        return float3(0.0f, 0.0f, 0.0f);

    // Half vector is the vector inbetween the light and view not light and normal.
    // You fool!
    float3 halfVector = normalize(viewDirection + lightDirection);
    float NoH = dot(halfVector, normal);
    float HoL = dot(halfVector, lightDirection);

    float D = D_GGXTrowbridgeReitz(NoH, roughness);
    float G = G_Smith_GGX(NoV, NoL, roughness);
    float3 F = F_Schlick(float3(0.04f, 0.04f, 0.04f), float3(1.0f, 1.0f, 1.0f), HoL);

    float attenuation = 1.0f; /// pow(length(lightDir), 2);
    float3 RDiffuse = (baseColor * NoL) / PI;
    float3 RSpec = D * G * F; // Divide by Pi in NDF

    return (RDiffuse + RSpec);
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
    // Get normalised vectors
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.cameraPosition - input.worldSpacePosition.xyz);

    float NoV = abs(dot(normal, viewDir)) + 1e-5f; // Avoid artifact
    
    float intensity = 1.0f;
    float3 materialDiffuse = material_diffuseColorAndOpacity.rgb;
    float3 emissiveColor = material_emissiveColor.rgb;
    float3 specularColor = material_specularColorAndSmoothness.rgb;
    float roughness = square(1.0f - material_specularColorAndSmoothness.a);

    // Since we are using SRGB formats for the textures the conversion to linear is done when reading
    if(material_hasDiffuseTexture == true)
        materialDiffuse = DiffuseTexture.Sample(DiffuseSampler, input.tex).rgb;
    if(material_hasEmissiveTexture == true)
        emissiveColor = EmissiveTexture.Sample(EmissiveSampler, input.tex).rgb;

    float3 radiance = 0.0f;
    [unroll]
    for (int i = 0; i < activeLights; i++)
    {
        Light light = lights[i];
        float3 lightDir = -normalize(light.position);
      
        radiance += EvaluateBRDF(normal, viewDir, lightDir, roughness, materialDiffuse) * light.color.rgb;
    }


    // Gamma encode
    return float4(GammaEncode(saturate(radiance)), 1.0f);
}


