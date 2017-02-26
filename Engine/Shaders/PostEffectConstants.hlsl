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
}