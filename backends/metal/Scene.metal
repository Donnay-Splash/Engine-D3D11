#include <metal_stdlib>

using namespace metal;

struct MeshUniforms {
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct MeshVertex {
    float4 position [[position]];
    float2 uv;
};

struct MeshVertexInput {
    float3 position [[attribute(0)]];
    float2 uv [[attribute(1)]];
};

vertex MeshVertex mesh_vertex(
    MeshVertexInput input [[stage_in]],
    constant MeshUniforms &uniforms [[buffer(0)]])
{
    MeshVertex output;
    const float4 worldPosition = uniforms.world * float4(input.position, 1.0);
    output.position = uniforms.projection * uniforms.view * worldPosition;
    output.uv = input.uv;
    return output;
}

fragment float4 mesh_fragment(MeshVertex input [[stage_in]])
{
    return float4(input.uv, 1.0 - input.uv.x, 1.0);
}

kernel void fill_values(
    device uint *values [[buffer(0)]],
    uint index [[thread_position_in_grid]])
{
    values[index] = index + 1;
}
