
// The expected format the vertex shader expects from the vertex buffers.
struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float4 worldSpacePosition : TEXCOORD1;
    float3 cameraPosition : TEXCOORD2;
};

struct GeometryOut
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float4 worldSpacePosition : TEXCOORD1;
    float3 cameraPosition : TEXCOORD2;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

// The format the pixel shader expects to receive from the vertex shader.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    float4 worldSpacePosition : TEXCOORD1;
    float3 cameraPosition : TEXCOORD2;
};