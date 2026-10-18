#include <metal_stdlib>

using namespace metal;

struct MeshUniforms {
    float4x4 model;
    float4x4 viewProjection;
    float4 diffuseColor;
};

struct MeshVertex {
    float4 position [[position]];
    float3 color;
};

struct MeshVertexInput {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
};

vertex MeshVertex mesh_vertex(
    MeshVertexInput input [[stage_in]],
    constant MeshUniforms &uniforms [[buffer(2)]])
{
    MeshVertex output;
    const float4 worldPosition = uniforms.model * float4(input.position, 1.0);
    const float3 worldNormal = normalize((uniforms.model * float4(input.normal, 0.0)).xyz);
    const float3 lightDirection = normalize(float3(-0.4, 0.7, -0.6));
    const float lighting = 0.25 + 0.75 * max(dot(worldNormal, lightDirection), 0.0);
    output.position = uniforms.viewProjection * worldPosition;
    output.color = uniforms.diffuseColor.rgb * lighting;
    return output;
}

fragment float4 mesh_fragment(MeshVertex input [[stage_in]])
{
    return float4(input.color, 1.0);
}
