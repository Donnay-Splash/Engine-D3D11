// The log of the maximum pixel offset before jumping down to the next lower mip level.
// If this number is too small (< 3), too many taps will land in the same pixel and we'll
// get bad variance that manifests as flashing.
// If it is too high (> 5) we'll get bad performance because we are not using the mip levels efficiently
#define LOG_MAX_OFFSET 3

// Must be less than or equal to the same macro defined in Engine.cpp
#define MAX_MIP_LEVEL 5

// Must be Less than MAX_MIP_LEVEL and >= 0
#define MIN_MIP_LEVEL 0

cbuffer GIConstants : register(b5)
{
    float numSamples; // Number of samples taken when calculating AO
    float numSpiralTurns; // Number of spirals to take when sampling. Best value of turns calculated in engine based on selected number of samples
    float aoRadius; // Ambient occlusion sphere radius in world-space
    float aoBias; // AO Bias to avoid darkening in smooth corners e.g. 0.01m

    float aoIntensity; // Controls the sharpness of the calculated AO
    float aoUseSecondLayer; // Interpolates between using one or two layers of deep G-Buffer
    float radiosityPropogationDamping; // Controls how much of the previous frames indirect lighting is propogated into the current frames
    float radiosityRadius;// Sampling radius for screen-space radiosity

    // Both values useful for artifically making colors stand out more in radiosity.
    float unsaturatedBoost; // How much to boost unsaturated colors
    float saturatedBoost; // How much to boost saturated colors.
    float radiosityContrastCentre;
    float envIntensity; // Multiplier controls how much the environment map adds to the final radiance
}


/*------------------------------
// Calculate the random rotation angle used when sampling pixels within the given radius
// Original hash function given in AlchemyAO paper presented at HPG12 : http://graphics.cs.williams.edu/papers/AlchemyHPG11/
------------------------------*/
float GetRandomRotationAngle(int2 pixelLocation)
{
    return (3 * pixelLocation.x ^ pixelLocation.y + pixelLocation.x * pixelLocation.y) * 10;
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
    float alpha = float(sampleNum + radialJitter) * (1.0f / numSamples);
    float angle = alpha * (numSpiralTurns * PI * 2.0f) + rotationAngle;

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
    return clamp(int(floor(log2(ssRadius))) - LOG_MAX_OFFSET, MIN_MIP_LEVEL, MAX_MIP_LEVEL);
}