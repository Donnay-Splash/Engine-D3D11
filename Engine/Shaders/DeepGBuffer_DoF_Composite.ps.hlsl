#include "DoFConstants.hlsl"
#include "PostEffectConstants.hlsl"

Texture2D farFieldSharpTexture : register(t0);
Texture2D nearFieldBlurredTexture : register(t1);
Texture2D farFieldBlurredTexture : register(t2);
SamplerState dofSampler : register(s0);

float3 PSMain(VertexOut input) : SV_Target0
{
    float4 sharpSample = farFieldSharpTexture.Sample(dofSampler, input.uv);
    float farFieldCoC = sharpSample.a;
    float3 sharpColour = sharpSample.rgb;

    float4 nearFieldSample = nearFieldBlurredTexture.Sample(dofSampler, input.uv);
    float nearFieldCoverage = nearFieldSample.a;
    float3 nearFieldColour = nearFieldSample.rgb;

    float3 farFieldSample = farFieldBlurredTexture.Sample(dofSampler, input.uv).rgb;
    
    farFieldCoC *= (farFieldCoC < 0.0f) ? farFieldRescale : 1.0f;

    if(farFieldCoC > 0.1f)
    {
        // Decrease far field CoC as it approaches the near field
        farFieldCoC = min(farFieldCoC * 1.5f, 1.0f);
    }
    

    // Lerp between far field and near field contribution.
    // Near field has pre-multiplied alpha
    return lerp(sharpColour, farFieldSample, abs(farFieldCoC)) * (1.0f - nearFieldCoverage) + nearFieldColour;
}