
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

VertexOut VSMain(VertexIn input)
{
    VertexOut output;
    output.position = input.position;
    output.uv = input.uv;

    return output;
}