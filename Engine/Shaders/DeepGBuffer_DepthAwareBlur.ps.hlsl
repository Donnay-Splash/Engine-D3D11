#include "System_Globals.hlsl"
#include "PostEffectConstants.hlsl"
#include "ReconstructFromDepth.hlsl"

// The input texture we are blurring
// We expect the texture to contain the
// AO in R and the depth key in G
Texture2D inputTexture : register(t0);
SamplerState textureSampler : register(s0);

#ifndef BILATERAL_PACKED
    Texture2D csZ : register(t1);
#endif

/*----------------------------------
Taken from G3D engine. Calculates the bilateral sampling weight to
maintain sharp edges across depth discontinuities. I.e. stops blurring 
between distant background and foreground objects

----------------------------------*/
float CalculateBilateralWeight(float mainKey, float tapKey)
{
    return max(0.0f, 1.0f - (blurEdgeSharpness * 2000.0f) * abs(tapKey - mainKey));


    // TODO: Can include normals to weight blurring for sharp corners
}

// ORIGINAL WIDTH
//#define BlurTaps 7

#define BALANCED

#ifdef HIGH_QUALITY
// PAPER: High Quality
#define BlurTaps 6
#endif

#ifdef BALANCED
// PAPER: Balanced
#define BlurTaps 5
#endif

#ifdef HIGH_PERFORMANCE
// PAPER: High Performance
#define BlurTaps 4
#endif

float4 PSMain(VertexOut input) : SV_Target
{
    // Weights for the blur // ORIGINAL WEIGHTS
    //const float weights[8] = { 0.197448, 0.174697, 0.120999, 0.065602, 0.02784, 0.009246, 0.002403, 0.000489 };

#ifdef HIGH_QUALITY
    // PAPER: High Quality
    const float weights[7] = { 0.261121, 0.210789, 0.110867, 0.037976, 0.008466, 0.001227, 0.000116 };
#endif

#ifdef BALANCED
    // PAPER: Balanced
    const float weights[6] = { 0.299485, 0.225985, 0.097049, 0.023688, 0.003279, 0.000257 };
#endif

#ifdef HIGH_PERFORMANCE
    // Paper: High Performance
    const float weights[5] = { 0.382928, 0.241732, 0.060598, 0.005977, 0.000229 };
#endif

    int2 fragCoord = input.position.xy;
    float4 sample = inputTexture.Load(int3(fragCoord, 0));

    float mainKey;
    float totalWeight = weights[0];

    #ifdef BILATERAL_PACKED
        float sum = 0.0f;
        sum += sample.r * totalWeight;
        mainKey = UnpackBilateralKey(sample.gb);
    #else   
        float4 sum = 0.0f;
        sum += sample * totalWeight;
        mainKey = csZ.Load(int3(fragCoord, 0)).r;
        mainKey = GetBilateralKey(mainKey);
    #endif

    if(mainKey <= 0.0f)
    {
        return 0.0f.xxxx;
    }

    [unroll] // For each sample grab a sample and add it to the sum
    for (int i = -BlurTaps; i < BlurTaps; i++)
    {
        if(i != 0)
        {
            int2 samplePos = fragCoord + blurAxis * i;
            float4 temp = inputTexture.Load(int3(samplePos, 0));
            float key;
            #ifdef BILATERAL_PACKED
            key = UnpackBilateralKey(temp.gb);
            float value = temp.r;
            #else
            key = csZ.Load(int3(samplePos, 0)).r;
            key = GetBilateralKey(key);
            float4 value = temp;
            #endif
            float weight = weights[abs(i)]; // do something with keys
            float bilateralWeight = CalculateBilateralWeight(mainKey, key);
            weight *= bilateralWeight;
            sum += value * weight;
            totalWeight += weight;
        }
    }

    const float epsilon = 0.0001f;
    #ifdef BILATERAL_PACKED
    float result = sum / (totalWeight + epsilon);
    return float4(result, sample.gb, 0.0f);
    #else
    float4 result = sum / (totalWeight + epsilon);
    return result;
    #endif
}