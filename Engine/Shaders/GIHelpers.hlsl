#include "lightingConstants.hlsl"

// The log of the maximum pixel offset before jumping down to the next lower mip level.
// If this number is too small (< 3), too many taps will land in the same pixel and we'll
// get bad variance that manifests as flashing.
// If it is too high (> 5) we'll get bad performance because we are not using the mip levels efficiently
#define LOG_MAX_OFFSET 3

// Must be less than or equal to the same macro defined in Engine.cpp
#define MAX_MIP_LEVEL 5


/*------------------------------
// Calculate the random rotation angle used when sampling pixels within the given radius
// Original hash function given in AlchemyAO paper presented at HPG12 : http://graphics.cs.williams.edu/papers/AlchemyHPG11/
------------------------------*/
float GetRandomRotationAngle(int2 pixelLocation)
{
    return ((3 * pixelLocation.x) ^ (pixelLocation.y + pixelLocation.x * pixelLocation.y)) * 10;
}

/*------------------------------------------
Returns a unit vector and screen-space radius on a unit disk.
This gives us our sample location. Caller must multiply by actual disk radius

Args:
sampleNum: How many samples have we taken so far. Determines how far we rotate.
rotationAngle: The calculated random rotation angle step we take for each sample
radialJitter: temporal jitter to apply to the sample rotation
OUT ssRadius: The calculated length to sample point along returned unit offset
------------------------------------------*/
float2 GetTapLocation(in int sampleNum, in float rotationAngle, in float radialJitter, out float ssRadius)
{
    // radius relative to ssRadius
    float alpha = float(sampleNum + radialJitter) * (1.0f / numAOSamples);
    float angle = alpha * (numAOSpiralTurns * PI * 2.0f) + rotationAngle;

    ssRadius = alpha;
    return float2(cos(angle), sin(angle));
}

/*------------------------------------------
Returns the correct mip level to sample from for the projected disk radius

Args:
ssRadius: screen-space sample radius in pixels
------------------------------------------*/
int GetMipLevel(float ssRadius)
{
    // Mip level = floor(log(ssr / MAX_OFFSET));
    return clamp(int(floor(log(ssRadius))) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);
}