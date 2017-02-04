/*  Contains all global constants for each pipeline stage used in deep G-Buffer generation */

// Previous depth buffer of first layer
// TODO: Select better registers as 1->n are for material textures
Texture2DArray previousDepth : register(t7);
SamplerState depthSampler : register(s7);

// Stores the view and projection transform matrices.
// TODO: These can be concatinated into a viewProjectionMatrix.
// TODO: Require previous viewProjection to calculate velocity vectors
cbuffer ViewBuffer : register(b0)
{
    // Matrices are set to row_major to match DirectXMath
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    float3 cameraPos;
    float padding;
    float3 clipInfo;
    float padding2;
    float2 invViewSize;
};

// Stores object information. e.g. object local to world transform.
cbuffer ObjectBuffer : register(b1)
{
    row_major matrix objectToCameraTransform;
    row_major matrix prevObjectToCameraTransform;
}

// Don't need lights here

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