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
    float blurEdgeSharpness; // Controls how sharp the edges are for the blur. A higher value increases sharpness but creates flickering at edges.

    float temporalBlendWeight; // Controls how much the current frame colour contributes to the final color
    float elapsedSceneTime; // Total elapsed time in seconds
    float aoEnabled; // Controls whether ambient occlusion is used in the final shading
    float radiosityEnabled; // Controls whether radiosity is used in final shading

    float sceneExposure; // Controls the whitepoint of the scene
    float3 pad; // padding

    float2 blurAxis; // Axis that we are blurring along. either [0, 1] for vertical or [1, 0] for horizontal
}