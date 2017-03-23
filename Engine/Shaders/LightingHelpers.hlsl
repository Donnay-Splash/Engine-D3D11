float square(float a)
{
    return a * a;
}

float3 GammaEncode(float3 linearColor)
{
    return pow(linearColor, kGammaEncodePower);
}

/*Can be used to positively or negatively weight colors*/
float ColorBoost(float3 color, float unsaturatedBoost, float saturatedBoost)
{
    if (unsaturatedBoost == saturatedBoost)
    {
        return unsaturatedBoost;
    }

    float ma = max(color.x, max(color.y, color.z));
    float mi = min(color.x, min(color.y, color.z));
    float saturation = (ma == 0.0f) ? 0.0f : ((ma - mi) / ma);

    return lerp(unsaturatedBoost, saturatedBoost, saturation);
}

// taken from http://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting
float3 FilmicAndGamma(float3 linearColor)
{
    float3 x = max(0, linearColor - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

// Remember final color = CustomFilmic(Linear color) / CustomFilmic(Linear white point value) : No gamma baked in
float3 CustomFilmic(float3 linearColor, float shoulderStr, float linearStr, float linearAngle, float toeStr, float toeNumer, float toeDenom)
{
    return ((linearColor * (shoulderStr * linearColor + linearAngle * linearStr) + toeStr * toeNumer) / (linearColor * (shoulderStr * linearColor + linearStr) + toeStr * toeDenom)) - toeNumer / toeDenom;
}

/*-----------------------------------------------------------------
    This function applies a "film-like" tonemap to supplied 
    HDR pixel. Does not apply 2.2 Gamma correction.
    Taken from Jim Hejl : https://twitter.com/jimhejl/status/841149752389558272

    Args:
    hdr: linear colour in HDR
    whitePoint: scene white point / exposure
-----------------------------------------------------------------*/
float3 ToneMapFilmic_Hejl2015(float3 hdr, float whitePoint)
{
    float4 vh = float4(hdr, whitePoint);
    float4 va = (1.425 * vh) + 0.05f; // evaluate filmic curve
    float4 vf = ((vh * va + 0.004f) / ((vh * (va + 0.55f) + 0.0491f))) - 0.0821f;
    return vf.rgb / vf.www;
}

float3 GammaDecode(float3 gammaColor)
{
    return pow(abs(gammaColor), kGammaDecodePower);
}

// Isotropic Normal Distribution Functions (NDF) 
// n = normal
// h = half normal
// a = perceptual roughness (roughness ^ 2)
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
    float GGXV = NoV + sqrt((NoV - NoV * a2) * NoV + a2);
    float GGXL = NoL + sqrt((NoL - NoL * a2) * NoL + a2);

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
    float3 RDiffuse = (baseColor) / PI;
    float3 RSpec = D * G * F; // Divide by Pi in NDF

    return (RDiffuse + RSpec) * NoL;
}

float3 EvaluateLambert(float3 normal, float3 lightDirection, float3 baseColor)
{
    float NoL = dot(normal, lightDirection);
    NoL = max(NoL, 0.0f);

    return (baseColor / PI) * NoL;
}

float SampleShadowWithOffset(float2 ssPos, float ssZ, float2 offset)
{
    float2 samplePoint = (ssPos + offset) / shadowMapDimensions;
    float shadowSample = shadowMap1.Sample(envSampler, samplePoint).r;
    return (shadowSample + SHADOWMAP_BIAS) < ssZ ? 0.0f : 1.0f;
}

// Applies pcf shadow filtering
float PCFFilter(float4 ssPos)
{
    float2 uvPos = (ssPos.xy * float2(0.5f, -0.5f)) + 0.5f;
    float2 ssPosPixels = uvPos * shadowMapDimensions;

    float shadowCoeff = SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(-1.5f, 0.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(0.5f, 0.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(-1.5f, -1.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(0.5f, -1.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(1.5f, -0.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(-0.5f, -0.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(1.5f, 1.5f)) +
                        SampleShadowWithOffset(ssPosPixels, ssPos.z, float2(-0.5f, 1.5f));

    return shadowCoeff * (1.0f / 8.0f);
}

float SampleShadowMapPCF(float3 csPosition)
{
    [branch]
    if (shadowsEnabled == 0.0f)
        return 1.0f;

    float4 ssPos = mul(float4(csPosition, 1.0f), shadowMapTransform);
    ssPos /= ssPos.w;
    
    float2 temp = abs(ssPos.xy);
    // Sample is outside of shadow map
    if (max(temp.x, temp.y) >= 1.0f)
        return 1.0f;

    return PCFFilter(ssPos);
}

float SampleShadowMapRaw(float3 csPosition)
{
    [branch]
    if (shadowsEnabled == 0.0f)
        return 1.0f;

    float4 ssPos = mul(float4(csPosition, 1.0f), shadowMapTransform);
    ssPos /= ssPos.w;
    
    float2 temp = abs(ssPos.xy);
    // Sample is outside of shadow map
    if (max(temp.x, temp.y) >= 1.0f)
        return 1.0f;

    float2 uvPos = (ssPos.xy * float2(0.5f, -0.5f)) + 0.5f;
    float2 ssPosPixels = uvPos * shadowMapDimensions;

    return SampleShadowWithOffset(ssPosPixels, ssPos.z, 0.0f);
}