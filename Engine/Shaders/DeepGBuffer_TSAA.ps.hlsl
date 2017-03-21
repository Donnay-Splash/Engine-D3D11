#include "PostEffectConstants.hlsl"
#include "Lighting.hlsl"
#include "System_Globals.hlsl"

Texture2D currentFrame : register(t0);
Texture2D previousFrame : register(t1);
Texture2DArray ssVelocityTexture : register(t2);
Texture2DArray depthTexture : register(t4);

SamplerState TSAASampler : register(s0);

cbuffer TSAAConstants : register(b4)
{
    float4 sampleWeights[3];
    float2 guardBandSize;
    float2 padddd;
    float currentFrameWeight;
}

float EncodeSSVelSq(float ssVelSq)
{
    return saturate(ssVelSq * 256.0f);
}

float DecodeSSVelSq(float value)
{
    return value / 256.0f;
}


/*-----------------------------------
Taken from UE4.
Intersects the ray from origin (Org) along direction (Dir)
with the AABB at {0, 0, 0} with dimensions (Box)
Returns the distance along the line segment
-----------------------------------*/
float IntersectAABB(float3 Dir, float3 Org, float3 Box)
{
    float3 RcpDir = rcp(Dir);
    float3 TNeg = (Box - Org) * RcpDir;
    float3 TPos = ((-Box) - Org) * RcpDir;
    return max(max(min(TNeg.x, TPos.x), min(TNeg.y, TPos.y)), min(TNeg.z, TPos.z));
}

/*----------------------------
Taken from UE4
Fast luminance approximation
----------------------------*/
float LumaFast(float3 color)
{
    return (color.g * 2.0f) + (color.r + color.b);
}

float HistoryClamp(float3 prevColor, float3 filtered, float3 neighborMin, float3 neighborMax)
{
    float3 Min = min(filtered, neighborMin);
    float3 Max = max(filtered, neighborMax);
    float3 Avg2 = Max + Min;
    float3 Dir = filtered - prevColor;
    float3 Org = prevColor - Avg2 * 0.5f;
    float3 Scale = Max - Avg2 * 0.5f;
    return saturate(IntersectAABB(Dir, Org, Scale));
}

static const float ssVelMaxLength = 0.003f;
static const int AA_CrossOffset = 2;



float4 PSMain(VertexOut input) : SV_Target
{
#if AA_PERFORMANCE
    float4 colorSample = currentFrame.Sample(TSAASampler, input.uv);
    float3 color = GammaDecode(colorSample.rgb);
    float2 ssVel = ssVelocity.Sample(TSAASampler, float3(input.uv, 0.0f));

    float ssVelLengthSq = dot(ssVel, ssVel) + 1e-9f;
    float4 prevFrameSample = previousFrame.Sample(TSAASampler, input.uv - ssVel);
    float3 prevColor = GammaDecode(prevFrameSample.rgb);
    float prevSSVelLen = DecodeSSVelSq(prevFrameSample.a);
    float blendWeight = 0.5f - 0.5f * saturate(ssVelLengthSq / ssVelMaxLength);
    blendWeight *= saturate(1.0f - abs(ssVelLengthSq - prevSSVelLen) * 4000.0f);

    color.rgb = lerp(color.rgb, prevColor, blendWeight);

    return float4(GammaEncode(color), EncodeSSVelSq(ssVelLengthSq));
#else

    // TODO: Add YCoCg clamping and HDR weighting
    float2 offset = guardBandSize;
    float2 offsetUV = (input.position.xy + offset) * invViewSize;
    float2 screenPos = (input.uv - 0.5f) * float2(2.0f, -2.0f); // converts uv range [0, 1] into screen coordinate range [-1.0f, 1.0f]
    float3 pixelPos;
    pixelPos.xy = offsetUV;
    pixelPos.z = depthTexture.Sample(TSAASampler, float3(offsetUV, 0.0f)).r;

    int2 velocitySampleOffset = int2(0.0f, 0.0f);
    float4 depths;

    depths = depthTexture.GatherRed(TSAASampler, float3(pixelPos.xy, 0.0f), int2(-AA_CrossOffset, -AA_CrossOffset), int2(AA_CrossOffset, -AA_CrossOffset), int2(-AA_CrossOffset, AA_CrossOffset), int2(AA_CrossOffset, AA_CrossOffset));

    float2 depthOffset = float2(AA_CrossOffset, AA_CrossOffset);
    float depthOffsetXx = float(AA_CrossOffset);

    [branch]
    if (depths.x < depths.y)
    {
        depthOffsetXx = -AA_CrossOffset;
    }
    if (depths.z < depths.w)
    {
        depthOffset.x = -AA_CrossOffset;
    }
    float minXY = min(depths.x, depths.y);
    float minZW = min(depths.z, depths.w);
    if (minXY < minZW)
    {
        depthOffset.y = -AA_CrossOffset;
        depthOffset.x = depthOffsetXx;
    }
    float minXYZW = min(minXY, minZW);
    if (minXYZW < pixelPos.z)
    {
        // This gives us the offset for reading from the velocity texture
        velocitySampleOffset = depthOffset * invViewSize;
        pixelPos.xy = offsetUV + velocitySampleOffset; // TODO: Might not need this
        pixelPos.z = minXYZW;
    }

    // We are trying to apply AA to an empty pixel.
    // return the current color
    if(pixelPos.z >= 1.0f)
    {
        return currentFrame.Sample(TSAASampler, offsetUV);
    }

    // Sample velocity from texture
    float2 ssVel = ssVelocityTexture.Sample(TSAASampler, float3(offsetUV + velocitySampleOffset, 0.0f)).rg;
    ssVel.y = -ssVel.y; // Flip Y since we are working in screen space and not UV space
    float2 prevSamplePos = ssVel;
    float2 temp = ssVel / invViewSize;

    #if !AA_BICUBIC // Want to use bicubic for best filtering results
    // Calculate the amount of blur from history based on velocity
    const float historyBlurAmp = 2.0f;
    float historyBlur = saturate(abs(temp.x) * historyBlurAmp + abs(temp.y) * historyBlurAmp);
    float velocity = sqrt(dot(temp, temp));
    #endif
    // converts back projection vector to [-1, 1] offset in viewport
    prevSamplePos = screenPos - prevSamplePos;
    bool offScreen = max(abs(prevSamplePos.x), abs(prevSamplePos.y)) >= 1.0f;

    // clamp projection to fit in viewport
    prevSamplePos.x = clamp(prevSamplePos.x, -1.0f + invViewSize.x, 1.0f - invViewSize.x);
    prevSamplePos.y = clamp(prevSamplePos.y, -1.0f + invViewSize.y, 1.0f - invViewSize.y);

    // WARNING: Probs wrong.
    prevSamplePos = prevSamplePos * float2(0.5f, -0.5f) + 0.5f;

    // Looks like YCoCg color space filtering provides better results
    // TODO: Use YCoCg space

    // Filter the pixel
    float4 Neighbor0 = currentFrame.Sample(TSAASampler, offsetUV, int2(-1, -1));
    float4 Neighbor1 = currentFrame.Sample(TSAASampler, offsetUV, int2(0, -1));
    float4 Neighbor2 = currentFrame.Sample(TSAASampler, offsetUV, int2(1, -1));
    float4 Neighbor3 = currentFrame.Sample(TSAASampler, offsetUV, int2(-1, 0));
    float4 Neighbor4 = currentFrame.Sample(TSAASampler, offsetUV);
    float4 Neighbor5 = currentFrame.Sample(TSAASampler, offsetUV, int2(1, 0));
    float4 Neighbor6 = currentFrame.Sample(TSAASampler, offsetUV, int2(-1, 1));
    float4 Neighbor7 = currentFrame.Sample(TSAASampler, offsetUV, int2(0, 1));
    float4 Neighbor8 = currentFrame.Sample(TSAASampler, offsetUV, int2(1, 1));

    // TODDO: Change to catmull-rom filtering
    // TODO: Remove low pass filter
    const float lowPassWeight = 1.0f / 9.0f;

    //TODO: If using HDR weight samples
    float4 Filtered = 
        Neighbor0 * sampleWeights[0].x +
        Neighbor1 * sampleWeights[0].y +
        Neighbor2 * sampleWeights[0].z +
        Neighbor3 * sampleWeights[0].w +
        Neighbor4 * sampleWeights[1].x +
        Neighbor5 * sampleWeights[1].y +
        Neighbor6 * sampleWeights[1].z +
        Neighbor7 * sampleWeights[1].w +
        Neighbor8 * sampleWeights[2].x;
    
    // Use unfiltered pixel for the 1 pixel edge of the screen
    float2 testPos = abs(screenPos);
    testPos += 2 * invViewSize;
    bool filteredOffScreen = max(testPos.x, testPos.y) >= 1.0f;
    if (filteredOffScreen)
    {
        Filtered = Neighbor4;
    }

    // Now find min and max of local neighborhood
    float4 neighborMin2 = min(min(Neighbor0, Neighbor2), min(Neighbor6, Neighbor8));
    float4 neighborMax2 = max(max(Neighbor0, Neighbor2), max(Neighbor6, Neighbor8));

    float4 neighborMin = min(min(Neighbor1, Neighbor3), min(Neighbor5, Neighbor7));
    float4 neighborMax = max(max(Neighbor1, Neighbor3), max(Neighbor5, Neighbor7));

    neighborMin2 = min(neighborMin2, neighborMin);
    neighborMax2 = max(neighborMax2, neighborMax);
    neighborMin = neighborMin * 0.5f + neighborMin2 * 0.5f;
    neighborMax = neighborMax * 0.5f + neighborMax2 * 0.5f;

    // Fetch the colour from the previous frame
    float4 prevSample = previousFrame.Sample(TSAASampler, prevSamplePos);
    float3 prevColor = prevSample.rgb;
    float prevVelocity = prevSample.a;

    // TODO: If using HDR weight sample

    // convert to luminocity to be used in blend weight calculation
    float lumaMin = LumaFast(neighborMin.rgb);
    float lumaMax = LumaFast(neighborMax.rgb);
    float lumaPrev = LumaFast(prevColor);
    float lumaContrast = lumaMax - lumaMin;

    // Clamp history using color AABB
    // Uses low-pass filtered color to avoid flickering.
    float clampedBlend = HistoryClamp(prevColor, Filtered.rgb, neighborMin.rgb, neighborMax.rgb);
    prevColor = lerp(prevColor, Filtered.rgb, clampedBlend);

    // Add back in some aliasing to sharpen overblurred edges
    float addAliasing = saturate(historyBlur) * 0.5f;
    float lumaContrastFactor = 32.0f;
    addAliasing = saturate(addAliasing + rcp(1.0f + lumaContrast * lumaContrastFactor));
    Filtered.rgb = lerp(Filtered.rgb, Neighbor4.rgb, addAliasing);
    float lumaFiltered = LumaFast(Filtered.rgb);

    // Compute blend weight //

    // Replace history with minimum of history from local neightborhood
    float distanceToClamp = min(abs(lumaMin - lumaPrev), abs(lumaMax - lumaPrev));
    float prevAmount = currentFrameWeight * 3.125 + historyBlur * (1.0f / 8.0f);
    float prevFactor = distanceToClamp * prevAmount * (1.0f + historyBlur * prevAmount * 8.0f);
    float blendFinal = saturate(prevFactor * rcp(distanceToClamp + lumaContrast));

    // Velocity weighting similar to low_quality method
    float velocityDecay = 0.5f;
    prevVelocity = max(prevVelocity * velocityDecay, velocity * rcp(velocityDecay));
    float velocityDiff = abs(prevVelocity - velocity) / max(1.0f, max(prevVelocity, velocity));
    blendFinal = max(blendFinal, velocityDiff * (1.0f / 8.0f));
    blendFinal = min(1.0f / 2.0f, blendFinal);

    // clamp offscreen pixels
    if (offScreen)
    {
        prevColor = Filtered.rgb;
        prevVelocity = 0.0f;
    }

    // Perform final blend of color and account for NaN and negative values
    float3 finalColor = lerp(prevColor.rgb, Filtered.rgb, blendFinal);
    finalColor.rgb = -min(-finalColor.rgb, 0.0f);

    return float4(finalColor, prevVelocity);
#endif
}

    /* Pseudo code for UE4 method

        Sample from depths in cross pattern  X O O O X   Where X are sample points and C is the current pixel
                                             O O O O O
                                             O O C O O
                                             O O O O O
                                             X O O O X
       
        // Take nearest depth of the samples including a central sample of the current pixel
        // Sample the ss velocity from the same location as the nearest depth.

        // Calculate a blur ammount from the sampled velocity. and clamp velocity so that it is not outside of the view

        // Filter the local neighborhood for the pixel. X X X where X is a sample point and C is the current pixel
                                                        X C X
                                                        X X X

        // If using HDR. Multiply the samples by a calculated HDR weight
        // Accumulate the samples using a kernel. Probably Gaussian. Then apply Low Pass using Low Pass weights
        // Test 1 pixel border to see if any of the neighborhood were outside of the current frame. If so use central sample.
        // Calculate Min and Max of neighborhood

        // Fetch the previous colour using calculated sample point.
        // If using HDR modify it by HDR weight

        // Calculate Luma for previous colour and min/max of neighbors
        // Clamp previous colour, using low pass to reduce flicker.

        // Add back in aliasing to sharpen

        // Compute blend ammount using both history luma and velocity decay similar to Crytek.
        // Convert any NaNs or negative colors to black. 
        // Using 1/8 weighting so looks like they are using 8 total samples. 
   
    End Pseudo code*/

