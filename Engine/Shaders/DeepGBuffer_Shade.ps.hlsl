// GBuffer layout
Texture2DArray diffuseColor : register(t0);
SamplerState gBufferSampler : register(s0);

Texture2DArray csNormals : register(t1);

Texture2DArray depth : register(t2);

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 PSMain(VertexOut input) : SV_Target
{
    return csNormals.Sample(gBufferSampler, float3(input.uv, 0.0f));
}