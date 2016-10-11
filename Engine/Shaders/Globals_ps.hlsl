// Helper class to store all pixel shader global constants for use in the uber shader
#include "InputStructs.hlsl"
#define MAX_LIGHT_COUNT 4
static const float PI = 3.14159265358979323846;

// Textures and samplers
Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

Texture2D SpecularTexture : register(t1);
SamplerState SpecularSampler : register(s1);

Texture2D EmissiveTexture : register(t2);
SamplerState EmissiveSampler : register(s2);

Texture2D NormalTexture : register(t3);
SamplerState NormalSampler : register(s3);

Texture2D SmoothnessTexture : register(t4);
SamplerState SmoothnessSampler : register(s4);

Texture2D OpacityTexture : register(t5);
SamplerState OpacitySampler : register(s5);

Texture2D AOTexture : register(t6);
SamplerState AOSampler : register(s6);

// struct containing light information
struct Light
{
    float3 position;
    float3 direction;
    float4 color;
};

// Contains description of the lights in the scene
cbuffer LightBuffer : register(b0)
{
    Light lights[MAX_LIGHT_COUNT];
    float activeLights;
};

cbuffer MaterialProperties : register(b1)
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