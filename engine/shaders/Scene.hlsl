cbuffer MeshUniforms : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct MeshVertexInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct MeshVertex
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

MeshVertex mesh_vertex(MeshVertexInput input)
{
    MeshVertex output;
    const float4 worldPosition = mul(world, float4(input.position, 1.0));
    output.position = mul(projection, mul(view, worldPosition));
    output.uv = input.uv;
    return output;
}

float4 mesh_fragment(MeshVertex input) : SV_Target0
{
    return float4(input.uv, 1.0 - input.uv.x, 1.0);
}

RWStructuredBuffer<uint> values : register(u0);

[numthreads(64, 1, 1)]
void fill_values(uint3 index : SV_DispatchThreadID)
{
    values[index.x] = index.x + 1;
}
