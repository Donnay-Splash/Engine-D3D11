#include "System_Globals.hlsl"
#include "PostEffectConstants.hlsl"
#include "ReconstructFromDepth.hlsl"
#include "LightingConstants.hlsl"
#include "GIHelpers.hlsl"

Texture2D LambertianTextureLayer1 : register(t0);
Texture2D LambertianTextureLayer2 : register(t1);
Texture2D PackedNormalsTexture : register(t2);
Texture2D csZTexture : register(t3);
SamplerState bufferSampler : register(s0);

/*-------------------------
Fetches the data from the textures for the given sample point.

Args:
ssSamplePoint: The screen-space position to sample the texture at. This will be conveted to u,v space [0, 1] before sampling
ssRadius: The distance from the current pixel to the sample point in pixels. This is used to select the best mip level
Out color0: Sampled color for the first layer
Out normal0: Sampled unit normal for the first layer
Out csPosition0: Reconstructed camera-space position of the sample point in the first layer
Out color1: Sampled color for the second layer
Out normal1: Sampled unit normal for the second layer
Out csPosition1 : Reconstructed camera-space position of the sampled point in the second layer
-------------------------*/
void GetSamplePointData(
    in float2 ssSamplePoint, 
    in float ssRadius, 
    out float3 color0, 
    out float3 normal0, 
    out float3 csPosition0, 
    out float3 color1, 
    out float3 normal1, 
    out float3 csPosition1)
{
    int mipLevel = GetMipLevel(ssRadius);
    //int2 ssTap = int2(ssSamplePoint) >> mipLevel;
    //int3 samplePoint = int3(ssTap, mipLevel);
    float2 samplePoint = ssSamplePoint * invViewSize;
    float4 normalSample = PackedNormalsTexture.SampleLevel(bufferSampler, samplePoint, mipLevel);
    float2 csZSample = csZTexture.SampleLevel(bufferSampler, samplePoint, mipLevel).rg;

    // Sampled data for first layer
    color0 = LambertianTextureLayer1.SampleLevel(bufferSampler, samplePoint, mipLevel).rgb;
    normal0 = normalize(UnpackNormal(normalSample.xy));
    csPosition0 = ReconstructCSPosition(ssSamplePoint, csZSample.x, projectionInfo);

    // Sampled data for second layer
    color1 = LambertianTextureLayer2.SampleLevel(bufferSampler, samplePoint, mipLevel).rgb;
    normal1 = normalize(UnpackNormal(normalSample.zw));
    csPosition1 = ReconstructCSPosition(ssSamplePoint, csZSample.y, projectionInfo);
}

/*-------------------------
Computes the indirect lighting at point X from the sampled point csPosition with given normal and radiosity.
Based on "A Deferred Shading Algorithm for Real-Time Indirect Illumination" by Soler, C., Hoel, O. and Rochet, F.
https://hal.inria.fr/inria-00480869v2/document
Modified for "Deep G-Buffers for Stable Global Illumination Approximation" by Mara, M., et al. 
http://graphics.cs.williams.edu/papers/DeepGBuffer16/

Indirect light (E) at point X given by equation
E = radiosity_Y * dot(w, csNormal_X)
Where w is the unit vector from X to Y
The calculated value only contributes if dot(w, n_x) > 0 and dot(w, n_y) < 0

Args:
csPosition_X: The camera-space positon of point X we are computing indirect light for
csNormal_X: The unit normal at point X
csPosition_Y: The camera-space position of point Y we are testing for contribution
csNormal_Y: The unit normal at point Y
radiosity_Y: The radiosity from point Y
Out irradiance: The calcuated irradiance at point X from Y 
Out weight: The contribution of this sample to the total indirect lighting 
-------------------------*/
void ComputeIndirectLightForPoint(
    in float3 csPosition_X, 
    in float3 csNormal_X, 
    in float3 csPosition_Y, 
    in float3 csNormal_Y, 
    in float3 radiosity_Y, 
    out float3 irradiance, 
    out float weight)
{
    float3 YminusX = csPosition_Y - csPosition_X;
    float3 w = normalize(YminusX);
    float WoNx = dot(w, csNormal_X);
    weight = ((WoNx > 0.0f) && (dot(-w, csNormal_Y) > 0.01f)) ? 1.0f : 0.0f;
    weight *= saturate(ceil((radiosityRadius * radiosityRadius) - dot(YminusX, YminusX)));


    irradiance = radiosity_Y * WoNx * weight;
}

/*-------------------------
Computes the indirect light at point X from a point within the sample radius (ssDiskRadius).

If the selected point contributes indirect lighting to point X then the total sum is incremented
and numSamplesUsed is incremented.

Otherwise if the selected point does not contribute the function returns not modifiying total irradiance
or numSamplesUsed

Args:
ssPosition: The screen space position of point X in pixels
csPosition: The camera-space position of the point X
csNormal: the unit normal at position X
ssDiskRadius: The size of the world-space sample radius projected onto the screen in pixels.
sampleIndex: The index of the current sample. Used to select sample offset from ssPosition within ssDiskRadius
randomRotationAngle: The calculated rotation angle to use for this pixel when selecting samples.
jitter: the temporal jitter to apply to selected rotation angle
Out irradianceSum: The total sum of indirect light contribution at point X
Out numSamplesUsed: The total number of samples that have contributed to the light at point X
-------------------------*/
void SampleIndirectLight(
    in float2 ssPosition,
    in float3 csPosition,
    in float3 csNormal,
    in float ssDiskRadius, 
    in int sampleIndex, 
    in float randomRotationAngle, 
    in float jitter, 
    inout float3 irradianceSum,
    inout float numSamplesUsed)
{
    float ssRadius;
    // Get the sample offset for this sample
    float2 sampleDirection = GetTapLocation(sampleIndex, randomRotationAngle, jitter, ssRadius);
    // Fit the unit length sample radius to the screen-space disk
    ssRadius *= ssDiskRadius;
    float2 ssSamplePoint = (ssRadius * sampleDirection) + ssPosition;

    // Get the data for both layers at the calcuated sample point
    float3 color0, normal0, csPosition0;
    float3 color1, normal1, csPosition1;
    GetSamplePointData(ssSamplePoint, ssRadius, color0, normal0, csPosition0, color1, normal1, csPosition1);

    float3 irradiance0;
    float weight0;
    ComputeIndirectLightForPoint(csPosition, csNormal, csPosition0, normal0, color0, irradiance0, weight0);
    float adjustedWeight0 = weight0 * dot(irradiance0, irradiance0) + weight0;

    float3 irradiance1;
    float weight1;
    ComputeIndirectLightForPoint(csPosition, csNormal, csPosition1, normal1, color1, irradiance1, weight1);
    float adjustedWeight1 = weight1 * dot(irradiance1, irradiance1) + weight1;

    float weight = (adjustedWeight0 > adjustedWeight1) ? weight0 : weight1;
    float3 irradiance = (adjustedWeight0 > adjustedWeight1) ? irradiance0 : irradiance1;

    numSamplesUsed += weight;
    irradianceSum += irradiance;
}

float4 PSMain(VertexOut input) : SV_Target
{
    float2 ssPosition = input.position.xy;
    float csZ = csZTexture.Sample(bufferSampler, input.uv).r;
    float3 csPosition = ReconstructCSPosition(input.position.xy, csZ, projectionInfo);
    float3 csNormal = UnpackNormal(PackedNormalsTexture.Sample(bufferSampler, input.uv).rg);

    [branch]
    if (csZ <= 0.0f)
    {
        return 0.0f.xxxx;
    }

    float ssDiskRadius = radiosityRadius * projectionScale / csPosition.z;
    float angle = GetRandomRotationAngle(int2(ssPosition)) + elapsedSceneTime;
    // Jitter applied for temporal accumulation of radiosity samples
    // Taken from G3D engine http://g3d.sourceforge.net/
    float radialJitter = frac(sin(ssPosition.x * 1e2 + elapsedSceneTime + ssPosition.y) * 1e5 + sin(ssPosition.y * 1e3) * 1e3) * 0.8f + 0.1f;

    float numSamplesUsed = 0.0f;
    float3 irradianceSum = 0.0f;
    for (int i = 0; i < numSamples; i++)
    {
        // Sample indirect lighting. Simples
        SampleIndirectLight(ssPosition, csPosition, csNormal, ssDiskRadius, i, angle, radialJitter, irradianceSum, numSamplesUsed);
    }

    float3 indirectResult = (irradianceSum * kSolidAngleHemisphere) / (numSamplesUsed + 0.00001f); // Minor offset avoid divide by zero
    indirectResult = -min(-indirectResult, 0.0f); // avoid NaNs
    float visibility = 1.0f - (numSamplesUsed / numSamples);

    return float4(indirectResult, visibility);
}