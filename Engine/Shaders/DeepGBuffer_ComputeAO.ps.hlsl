#include "PostEffectConstants.hlsl"
#include "System_Globals.hlsl"
#include "ReconstructFromDepth.hlsl"

//constants
// Minimum AO radius is 3 pixels.
#define MIN_RADIUS 3
#define PI 3.14159265358979323846

// The log of the maximum pixel offset before jumping down to the next lower mip level.
// If this number is too small (< 3), too many taps will land in the same pixel and we'll
// get bad variance that manifests as flashing.
// If it is too high (> 5) we'll get bad performance because we are not using the mip levels efficiently
#define LOG_MAX_OFFSET 3

// Must be less than or equal to the same macro defined in Engine.cpp
#define MAX_MIP_LEVEL 5

Texture2D csZTexture : register(t0);
SamplerState csZSampler : register(s0);


float square(float val)
{
    return val * val;
}

/*------------------------------------------
Returns a unit vector and screen-space radius on a unit disk.
This gives us our sample location. Caller must multiply by actual disk radius

Args:
sampleNum: How many samples have we taken so far. Determines how far we rotate.
rotationAngle: The calculated random rotation angle step we take for each sample
OUT ssRadius: The calculated length to sample point along returned unit offset
------------------------------------------*/
float2 GetTapLocation(in int sampleNum, in float rotationAngle, out float ssRadius)
{
    // radius relative to ssRadius
    float alpha = float(sampleNum + 0.5f) * (1.0f / numAOSamples);
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

/*------------------------------------------
Returns the two sampled camera-space position for first and second layer. Based on position of sample location and circular offset

Args:
ssPosition: position of current fragment in non-normalised screen space
unitOffset: unit screen-space direction from sample location
ssRadius: radius of disk to sample from. SampleLocation = ssPosition + offset*ssRadius
inverseScale: Scales the calculated texture coordinates to the normalised [0, 1] range
OUT firstLayerPos: sampled camera-space location from first layer of deep G-Buffer
OUT secondLayerPos: sampled camera-space location from second layer of deep G-Buffer
------------------------------------------*/
void SampleOffsetPositions(int2 ssPosition, float2 unitOffset, float ssRadius, float2 invScale, out float3 firstLayerPos, out float3 secondLayerPos)
{
    int mipLevel = GetMipLevel(ssRadius);
    float2 ssSamplePoint = float2(ssRadius * unitOffset) + ssPosition;
    float2 normalisedSamplePoint = float2(ssSamplePoint) * invScale;

    // Calculate mip position based on downsample
    float2 Zs = csZTexture.SampleLevel(csZSampler, normalisedSamplePoint, mipLevel).rg;

    // Return sampled positions offset to pixel centre
    firstLayerPos = ReconstructCSPosition(float2(ssSamplePoint), Zs.x, projectionInfo);
    secondLayerPos = ReconstructCSPosition(float2(ssSamplePoint), Zs.y, projectionInfo);
}

/*------------------------------------------
Selection of falloff functions to control contrast of calculated AO term.

Args:
vv: squared vector from camera point to sampled point
vn: dot product between vector to sampled point and the normal of the camera point
epsilon: avoids division by zero. Note a higher epsilon will avoid over darkening within cracks
------------------------------------------*/
float FallOff(float vv, float vn, float epsilon)
{
    float radius2 = square(aoRadius);
    float invRadius2 = rcp(radius2);
    // Original falloff function from AlchemyAO paper:http://graphics.cs.williams.edu/papers/AlchemyHPG11/
    // presumes the desired result is intensity/radius^6
    // return float(vv < radius2) * max((vn - aoBias) / (epsilon + vv), 0.0f) * radius2 * 0.6f;

    // Smoother transition to zero (lowers contrast, smoothing out corners). 
    // From Scalable Ambient Obscurance: http://graphics.cs.williams.edu/papers/SAOHPG12/
    float f = max(1.0f - vv * invRadius2, 0.0f);
    return f * max((vn - aoBias) * rsqrt(epsilon + vv), 0.0f);

    // Lower quality than above but faster
    // Assumes desired result is intensity/radius^6
    //float f = max(radius2 - vv, 0.0f);
    //return f * f * f * max((vn - aoBias) / (epsilon + vv), 0.0f);

    // Medium contrast (Looks better at higher radii)
    // Note that contribution falls off with radius ^ 2
    // Assumes division by radius^6 in PSMain()
    // return 4.0f * max(1.0f - vv * invRadius2, 0.0f) * max(vn - aoBias, 0.0f);

    // Low contrast
    // return 2.0f * float(vv < radius2) * max(vn - aoBias, 0.0f);
}


/*------------------------------------------
Returns the [0, 1] occlusion value calculated using the AlchemyAO equation. http://graphics.cs.williams.edu/papers/AlchemyHPG11/

Args:
csSampledPosition: The position we have sampled that we want to test for occlusion
csPosition: The position we are calculating occlusion for
csNormal: The calculated face normal at csPosition
------------------------------------------*/
float AOForPosition(float3 csSampledPosition, float3 csPosition, float3 csNormal)
{
    float3 v = csSampledPosition - csPosition;
    float vv = dot(v, v);
    float vn = dot(v, csNormal);
    // Epsilon value to avoid divide by 0
    const float epsilon = 0.001f;

    // Angular correction
    float ac = lerp(1.0f, max(0.0f, 1.5f * -csNormal.z), 0.35f);

    // Without angular correction surfaces seen head on have less AO
    return FallOff(vv, vn, epsilon) * ac;
}


/*------------------------------------------
Compute the occlusion for the given sample at the given screen space position

Args:
ssPosition: The screen-space position of current fragment in pixels [0, width-1] [0, height-1]
rotationAngle: The random rotation angle to sample around the given camera-space position
csPosition: The position of the point we are calculating AO for in camera-space
csNormal: The face normal for the give position in camera-space
ssDiskRadius: The maximum screen space radius to read our samples from
sampleNum: What sample are we calculating for. Used to offset sample location by rotation angle
invScale: inverse scale of the texture size to convert from explicit texture location to normalised sample point
------------------------------------------*/
float CalculateOcclusion(int2 ssPosition, float rotationAngle, float3 csPosition, float3 csNormal, float ssDiskRadius, int sampleNum, float2 invScale)
{
    float ssRadius;
    float2 tapOffset = GetTapLocation(sampleNum, rotationAngle, ssRadius);
    
    // Ensure that the taps are separated by at least one pixel
    ssRadius = max(0.75f, ssRadius * ssDiskRadius);

    float3 firstLayerPos, secondLayerPos;
    SampleOffsetPositions(ssPosition, tapOffset, ssRadius, invScale, firstLayerPos, secondLayerPos);

    // Return the max value. i.e. layer with least occlusion
    float AO1 = AOForPosition(firstLayerPos, csPosition, csNormal);
    float AO2 = AOForPosition(secondLayerPos, csPosition, csNormal);

    return lerp(AO1, max(AO1, AO2), aoUseSecondLayer);
    //return max(AOForPosition(firstLayerPos, csPosition, csNormal), AOForPosition(secondLayerPos, csPosition, csNormal));
}

/*------------------------------------------
Packs camera space Z into [0, 1] range for use in depth aware blur.
Note: clipInfo.z = farPlane

Args:
csZ: camera-space z coordinate of current pixel
------------------------------------------*/
float GetBilateralKey(float csZ)
{
    return clamp(csZ * rcp(clipInfo.z), 0.0f, 1.0f);
}


float4 PSMain(VertexOut input) : SV_Target
{ 
    float visibility = 1.0f;
    // Sample camera space Z for the current pixel
    float csZ = csZTexture.Sample(csZSampler, input.uv).r;
    // reconstruct cs position for current pixel
    float3 csPosition = ReconstructCSPosition(input.position.xy, csZ, projectionInfo);

    float bilateralKey = GetBilateralKey(csZ);

    uint height, width;
    csZTexture.GetDimensions(width, height);
    float2 invSize = 1.0f / float2(width, height);

    // Calculate normal for face using derivatives
    float3 cs_n = ReconstructNonUnitFaceNormal(csPosition);
    // If normal is at depth discontinuity e.g. corner of object
    // Since cs_n is computed using the cross product of camera-space edge vectors from points at adjacent pixels,
    // its magnitude will be proportional to the square of distance from the camera
    // If the threshold number(0.00006) is too big we will see black dots where we have used a bad normal.
    // Too small and we will see white dots
    if(dot(cs_n, cs_n) > square(csPosition.z * csPosition.z * 0.00006f))
    {
        // The normals from depth should be very small values before normalisation (since we are using x,y derivatives)
        // except at depth discontinuities, where they will be large and lead to 1-pixel false occlusions
        // because they are not reliable.


        // return full visibility
        return float4(visibility, bilateralKey, 0.0f, 0.0f);
    }
    else
    {
        // Normalise calculated normal
        cs_n = normalize(cs_n);
    }

    // Calculate the random rotation angle used when sampling pixels within the given radius
    // Original hash function given in AlchemyAO paper presented at HPG12 : http://graphics.cs.williams.edu/papers/AlchemyHPG11/
    int2 integerPixelLocation = int2(input.position.xy);
    float angle = ((3 * integerPixelLocation.x) ^ (integerPixelLocation.y + integerPixelLocation.x*integerPixelLocation.y)) * 10;

    // Calculate screen space disk radius based on projected size of sample sphere
    float ssDiskRadius = projectionScale * aoRadius / csPosition.z;

    if(ssDiskRadius < MIN_RADIUS)
    {
        // Radius is too small to calculate accurate AO
        // Return full visibility

        return float4(visibility, bilateralKey, 0.0f, 0.0f);
    }


    float aoSum = 0.0f;
    // For number of samples
    for (int i = 0; i < numAOSamples; i++)
    {
    // Calculate next sample location
    // Calculate required mip level
    // sample csZ
    // Calculate csPosition
    // Calculate A
    // Sum A
        aoSum += CalculateOcclusion(integerPixelLocation, angle, csPosition, cs_n, ssDiskRadius, i, invSize);
    }
    // High quality normalisation
    float A = pow(max(0.0f, 1.0f - sqrt(aoSum * (3.0f / numAOSamples))), aoIntensity);

    // Low quality normalisation
    //float A = max(0.0f, 1.0f - sum * (aoIntensity/radius^6) * (5.0f / numAOSamples));
    // Anti-tone map to reduce contrast and make dark regions darker
    //A = (pow(A, 0.2f) + 1.2f * A*A*A*A) / 2.2f;

    // Visualise random rotation angle distribution
    // A = frac(angle / (PI * 2.0f));

    // Fade as the radius approaches the minimum radius
    visibility = lerp(1.0f, A, saturate(ssDiskRadius - MIN_RADIUS));

    // Divide total sum by num samples
    // return calculated A
    return float4(visibility, bilateralKey, 0.0f, 0.0f);
}