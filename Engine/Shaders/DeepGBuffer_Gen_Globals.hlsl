/*  Contains all global constants for each pipeline stage used in deep G-Buffer generation */
#include "System_Globals.hlsl"

// Textures and samplers
Texture2D DiffuseTexture : register(t0);
SamplerState MaterialSampler : register(s0);

Texture2D SpecularTexture : register(t1);

Texture2D EmissiveTexture : register(t2);

Texture2D NormalTexture : register(t3);

Texture2D SmoothnessTexture : register(t4);

Texture2D OpacityTexture : register(t5);

Texture2D AOTexture : register(t6);

// Previous depth buffer of first layer
// TODO: Select better registers as 1->n are for material textures
Texture2DArray previousDepth : register(t7);
SamplerState depthSampler : register(s7);

// Stores object information. e.g. object local to world transform.
cbuffer ObjectBuffer : register(b1)
{
    row_major matrix objectToWorldTransform;
    row_major matrix objectToCameraTransform;
    row_major matrix prevObjectToCameraTransform;
}

cbuffer DeepGBufferConstants : register(b2)
{
    float minSeparation;
}

cbuffer MaterialBuffer : register(b3)
{
    float4 material_diffuseColorAndOpacity;
    float4 material_specularColorAndSmoothness;
    float4 material_emissiveColor;
    bool material_hasDiffuseTexture;
    bool material_hasSpecularTexture;
    bool material_hasEmissiveTexture;
    bool material_hasNormalTexture;
    bool material_hasSmoothnessTexture;
    bool material_hasOpacityTexture;
    bool material_hasAOTexture;
}