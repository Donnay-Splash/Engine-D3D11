static const float PI = 3.14159265358979323846;
static const float kGammaEncodePower = 1.0f / 2.2f;
static const float kGammaDecodePower = 2.2f;
static const float kSolidAngleHemisphere = 2.0f * PI;

#define MAX_LIGHT_COUNT 4
#define MAX_ENV_MIP_LEVEL 20
TextureCube environmentMap : register(t10);
SamplerState envSampler : register(s10);
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
    float environmentMapMips;
    float environmentMapEnabled;
}