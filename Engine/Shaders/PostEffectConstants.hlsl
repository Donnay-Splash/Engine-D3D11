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
    float basicTemporalFilterBlend; // How much of the previous frame to blend with the current frame. Defaults to 0.9f
    float numAOSamples; // Number of samples taken when calculating AO
    float numAOSpiralTurns; // Number of spirals to take when sampling. Best value of turns calculated in engine based on selected number of samples
    float aoRadius; // Ambient occlusion sphere radius in world-space
    float aoBias; // AO Bias to avoid darkening in smooth corners e.g. 0.01m
    float aoIntensity; // Controls the sharpness of the calculated AO
    float aoUseSecondLayer; // Interpolates between using one or two layers of deep G-Buffer
    float blurEdgeSharpness; // Controls how sharp the edges are for the blur. A higher value increases sharpness but creates flickering at edges.
    float temporalBlendWeight; // Controls how much the current frame colour contributes to the final color
    float elapsedSceneTime; // Total elapsed time in seconds
    // TODO: Figure out padding
    float2 blurAxis; // Axis that we are blurring along. either [0, 1] for vertical or [1, 0] for horizontal
}