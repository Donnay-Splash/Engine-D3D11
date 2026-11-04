cbuffer MeshUniforms : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

// Space 0 is engine owned: per-draw constants and the engine's fixed samplers.
SamplerState linearRepeat : register(s0, space0);

// Space 1 holds material parameters: one constant buffer plus textures. The compiler
// reflects these so materials can set them by name.
cbuffer MaterialParams : register(b0, space1)
{
    bool useTexture; // @default(true)
    int mode; // @default(0) 0 plain, 1 grayscale
    float intensity; // @default(1)
    float4 tint; // @default(1, 1, 1, 1)
    float2 uvScale; // @default(1, 1)
};

Texture2D baseTexture : register(t0, space1); // @default(white)

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
    float4 color = float4(input.uv, 1.0 - input.uv.x, 1.0);
    if (useTexture) {
        color = baseTexture.Sample(linearRepeat, input.uv * uvScale);
    }
    if (mode == 1) {
        color.rgb = dot(color.rgb, float3(0.299, 0.587, 0.114)).xxx;
    }
    return float4(color.rgb * tint.rgb * intensity, tint.a);
}

RWStructuredBuffer<uint> values : register(u0);

[numthreads(64, 1, 1)]
void fill_values(uint3 index : SV_DispatchThreadID)
{
    values[index.x] = index.x + 1;
}
