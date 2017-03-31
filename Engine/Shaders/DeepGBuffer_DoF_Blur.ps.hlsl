/*  Splits the scene into near and far fields depending on depth and DoF settings*/

#include "PostEffectConstants.hlsl"
#include "DoFConstants.hlsl"

// As input takes packed colour and CoC alongside near and far field images
Texture2D nearFieldTexture : register(t0);
Texture2D farFieldTexture : register(t1);
Texture2D CoCTexture : register(t2);
SamplerState dofSampler : register(s0);

struct PixelOut
{
    float4 halfResNearField : SV_Target0;
    float4 halfResFarField : SV_Target1;
};

PixelOut PSMain(VertexOut input)
{
    PixelOut result;

    const int KERNEL_TAPS = 6;
    float kernel[KERNEL_TAPS + 1];
    float2 fragCoord = input.position.xy;
    float2 textureDimensions;
    nearFieldTexture.GetDimensions(textureDimensions.x, textureDimensions.y);

    float2 samplePointPixels = input.uv * textureDimensions;
    float2 CoCSamplePoint = input.uv * CoCTextureDimensions;

    // Uses same Kernel used for DoF in Skylanders SWAP Force : http://casual-effects.blogspot.co.uk/2013/09/the-skylanders-swap-force-depth-of.html 
    // 11 x 11 kernel. Does not dictate DoF weights but is scaled to fit a CoC based kernel at each pixel
    kernel[6] = 0.00f;
    kernel[5] = 0.50f;
    kernel[4] = 0.60f;
    kernel[3] = 0.75f;
    kernel[2] = 0.90f;
    kernel[1] = 1.00f;
    kernel[0] = 1.00f;

    float4 farFieldColourSum = 0.0f;
    float farFieldWeightSum = 0.0f;

    float4 nearFieldColourSum = 0.0f;
    float nearFieldWeightSum = 0.0f;

	// TODO: Use this when sampling for near field blur.
	// If tap is outside of nearFieldCoC then it does not contribute.
	// Hopefully this should spread blur out into dark regions.
    float2 CoCSample = CoCTexture.Sample(dofSampler, input.uv).rg;
    float nearFieldCoC = CoCSample.r;
    float nearFieldRadius = nearFieldCoC * maxCoCRadiusPixels;
    float invNearFieldRadius = 1.0f / nearFieldRadius;

    float farFieldCoC = CoCSample.g;
    float farFieldRadius = farFieldCoC * maxCoCRadiusPixels;

    for (int i = -maxCoCRadiusPixels; i <= maxCoCRadiusPixels; ++i)
    {
        ////////////////////////////////////////
        // Caculate the sum for the far field //
        ////////////////////////////////////////
         
        float2 tapLocation = samplePointPixels + (blurAxis * i);
        float2 CoCTap = CoCSamplePoint + (blurAxis * i);
        float2 normalisedTapLocation = tapLocation / textureDimensions;
        float2 normalisedCoCTap = CoCTap / CoCTextureDimensions;
        float2 tapCoC = CoCTexture.Sample(dofSampler, normalisedCoCTap).rg;

        // Blur far-field image
        float4 farFieldInput = farFieldTexture.Sample(dofSampler, normalisedTapLocation);
        float farFieldTapRadius = tapCoC.g * maxCoCRadiusPixels;

        float weight =
            // Only want to blur tap over current pixel if it is closer to the viewer. i.e. If taps CoC is < farFieldCoC
            //float(farFieldTapRadius <= farFieldRadius) * // TODO: Look at optimising this
        
            // Stretch kernel to fit radius at tap location
            kernel[clamp(int(floor(abs(i) * (KERNEL_TAPS - 1)) / (0.001f + abs(farFieldTapRadius * 0.8f))), 0.0f, KERNEL_TAPS)];

        farFieldWeightSum += weight;
        farFieldColourSum.rgb += farFieldInput.rgb * weight;


        /////////////////////////////////////////
        // Caculate the sum for the near field //
        /////////////////////////////////////////

        // Need to split into two passes
        // On the first pass we take the CoC values from the alpha
        // channel to calculate coverage. Storing that coverage in alpha
        // On the second pass we no longer need CoC so instead take coverage from alpha.

        // Get the tap from the near field image
        float4 nearFieldTap = nearFieldTexture.Sample(dofSampler, normalisedTapLocation);
        float nearFieldWeight = float(abs(i) <= abs(nearFieldRadius));
        #ifdef HORIZONTAL
        // For first horizontal pass calculate coverage from tap CoC

        // Calculate the blur radius from the signed CoC
        float nearFieldTapRadius = tapCoC.r * maxCoCRadiusPixels;

        // Calculate coverage based on sample. I think we want CoC for dark near field values to be 0 as to not contribute to blur maybe? idk
        nearFieldTap.a = float(abs(i) <= nearFieldTapRadius) * saturate(nearFieldTapRadius * abs(invNearFieldRadius) * 4.0f); //Note: this might need to be changed depedning on sign of nearFieldRadius;
        // Optionally increase fade at near field edge
        // nearFieldTap.a *= nearFieldTap.a; nearFieldTap.a *= nearFieldTap.a;

        // Compute pre-multiplied alpha colour
        nearFieldTap.rgb *= nearFieldTap.a;
        #endif

        // According to research carried out for SWAP Force DoF the more complex kernel function is
        // not worth the additional cost here so instead we use a simple box filter
        nearFieldColourSum += nearFieldTap * nearFieldWeight;
        nearFieldWeightSum += nearFieldWeight;
    }

    // Store CoC for far field for next pass
    result.halfResFarField.a = farFieldCoC;
    // Normalise the result based on weight
    result.halfResFarField.rgb = farFieldColourSum.rgb / farFieldWeightSum;

    result.halfResNearField = nearFieldColourSum / max(nearFieldWeightSum, 0.001f);

    // Return result ready for blur in other axis
    return result;
}