#include "PostEffectConstants.hlsl"
#include "Lighting.hlsl"

Texture2D HDRInput : register(t0);
SamplerState hdrSampler : register(s0);

float3 PSMain(VertexOut input) : SV_Target
{
    float3 hdrVal = HDRInput.Sample(hdrSampler, input.uv).rgb;
    float exp = max(0.001f, sceneExposure);
    float whitePoint = 1.0f / exp;
    float3 linearVal = ToneMapFilmic_Hejl2015(hdrVal, whitePoint);

    // Finally gamma encode final value
    return GammaEncode(saturate(linearVal));
}
