// Helper class to store all pixel shader global constants for use in the uber shader
#include "InputStructs.hlsl"
#define MAX_LIGHT_COUNT 4

// Textures and samplers
Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

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
    float4 diffuseColorAndOpacity;
    float4 specularColorAndSmoothness;
}