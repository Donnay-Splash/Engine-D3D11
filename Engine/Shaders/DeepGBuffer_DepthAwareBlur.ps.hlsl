#include "PostEffectConstants.hlsl"

// The input texture we are blurring
// We expect the texture to contain the
// AO in R and the depth key in G
Texture2D inputTexture : register(t0);
SamplerState textureSampler : register(s0);

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

#define BlurTaps 7

float2 PSMain(VertexOut input) : SV_Target
{
    // Weights for the blur
    const float weights[8] = { 0.197448, 0.174697, 0.120999, 0.065602, 0.02784, 0.009246, 0.002403, 0.000489 };

    int2 fragCoord = input.position.xy;
    float4 sample = inputTexture.Load(int3(fragCoord, 0));

    float sum = 0.0f;
    float totalWeight = weights[0];
    sum += sample.r * totalWeight;
    float mainKey = sample.g;

    [unroll] // For each sample grab a sample and add it to the sum
    for (int i = -BlurTaps; i < BlurTaps; i++)
    {
        if(i != 0)
        {
            int2 samplePos = fragCoord + blurAxis * i;
            float4 temp = inputTexture.Load(int3(samplePos, 0));
            float key = temp.g;
            float value = temp.r;
            float weight = weights[abs(i)]; // do something with keys
            float bilateralWeight = CalculateBilateralWeight(mainKey, key);
            weight *= bilateralWeight;
            sum += value * weight;
            totalWeight += weight;
        }
    }

    const float epsilon = 0.0001f;
    float result = sum / (totalWeight + epsilon);
    return float2(result, mainKey);
}