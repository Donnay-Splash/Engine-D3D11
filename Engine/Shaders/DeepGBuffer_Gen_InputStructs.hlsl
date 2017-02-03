/*  Contains all input structs for each pipeline stage for deep G-buffer generation*/

struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct GeometryInput
{
    float4 position : SV_Position;
    float3 csNormal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    float4 csPosition : POSITION0;
    float4 csPrevPosition : POSITION1;
};

struct PixelInput
{
    float4 position : SV_Position;
    float3 csNormal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    float4 csPosition : POSITION0;
    float4 csPrevPosition : POSITION1;
    uint renderTargetIndex : SV_RenderTargetArrayIndex;
};