struct VertexIn
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer Constants : register(b1)
{
    float displaySecondLayer;
    float gBufferTarget;
    float currentMipLevel;
    float numAOSamples; // Number of samples taken when calculating AO
    float numAOSpiralTurns; // Number of spirals to take when sampling. Best value of turns calculated in engine based on selected number of samples
    float aoRadius; // Ambient occlusion sphere radius in world-space
    float aoBias; // AO Bias to avoid darkening in smooth corners e.g. 0.01m
    float aoIntensity; // Controls the sharpness of the calculated AO
    float aoUseSecondLayer; // Interpolates between using one or two layers of deep G-Buffer
}