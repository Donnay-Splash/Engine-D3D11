/*  Splits the scene into near and far fields depending on depth and DoF settings*/

#include "PostEffectConstants.hlsl"
#include "DoFConstants.hlsl"

// As input take camera space Z and shaded scene
Texture2D frontLayerShadedTexture : register(t0);
Texture2D backLayerShadedTexture : register(t1);
Texture2D csZTexture : register(t2);
SamplerState dofSampler : register(s0);

struct PixelOut
{
    float3 nearPlane : SV_Target0;
    float3 farPlane : SV_Target1;
    float2 CoC : SV_Target2;
};

float GetCoCRadius(float z)
{
    float radius;
    if(z <= 0)
    {
        radius = 0.0f;
    }
    else if (z < nearSharpPlaneZ)
    {
        // In front of focus area
        radius = (nearSharpPlaneZ - max(z, nearBlurryPlaneZ)) * nearScale; // CoC [0, 1]
    }
    else if (z < farSharpPlaneZ)
    {
        // In focus area
        radius = 0.0f;
    }
    else
    {
        // behind focus area
        radius = (farSharpPlaneZ - min(z, farBlurryPlaneZ)) * farScale; // CoC [-1, 0]
    }

    return radius;
}

PixelOut PSMain(VertexOut input)
{
    PixelOut result;

    float3 frontLayerColour = frontLayerShadedTexture.Sample(dofSampler, input.uv).rgb;
#ifdef DOF_DEEP
    float3 backLayerColour = backLayerShadedTexture.Sample(dofSampler, input.uv).rgb;
#endif
    float2 zSample = csZTexture.Sample(dofSampler, input.uv);
    float frontLayerRadius = GetCoCRadius(zSample.r);
#ifdef DOF_DEEP
    float backLayerRadius = GetCoCRadius(zSample.g);
#endif

    float blend = smoothstep(0.0f, 0.05f, frontLayerRadius);

    // TODO: Mix in second layer into far plane colour for areas of low blend
    // TODO: When deep G-Buffer blending not enabled. store near field colour
    //       else store deep G-Buffer data

#ifdef DOF_DEEP
    float3 farPlaneFillColour = lerp(frontLayerColour, backLayerColour, useSecondLayer);
    float fillRadius = lerp(frontLayerRadius, backLayerRadius, useSecondLayer);
#else
    float3 farPlaneFillColour = frontLayerColour;
    float fillRadius = frontLayerRadius;
#endif
    float farFillFactor = floor(blend);
    float nearBlendFactor = ceil(blend);

    result.nearPlane.rgb = nearBlendFactor * frontLayerColour;
    result.CoC.r = lerp(-1.0f, frontLayerRadius, nearBlendFactor);
    result.farPlane.rgb = lerp(frontLayerColour, farPlaneFillColour, farFillFactor);
    result.CoC.g = lerp(frontLayerRadius, fillRadius, farFillFactor);

    return result;
}