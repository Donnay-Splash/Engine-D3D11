static const float PI = 3.14159265358979323846;
static const float kGammaEncodePower = 1.0f / 2.2f;
static const float kGammaDecodePower = 2.2f;

#define MAX_LIGHT_COUNT 4
// struct containing light information
struct Light
{
    float3 position;
    float3 direction;
    float4 color;
};

// Contains description of the lights in the scene
cbuffer LightBuffer : register(b2)
{
    Light lights[MAX_LIGHT_COUNT];
    float activeLights;
}