#include <metal_stdlib>

using namespace metal;

struct TriangleVertex {
    float4 position [[position]];
    float3 color;
};

vertex TriangleVertex triangle_vertex(uint vertexID [[vertex_id]])
{
    const float2 positions[3] = {
        float2(0.0, 0.65),
        float2(-0.65, -0.55),
        float2(0.65, -0.55)
    };
    const float3 colors[3] = {
        float3(1.0, 0.25, 0.2),
        float3(0.2, 0.85, 0.4),
        float3(0.2, 0.5, 1.0)
    };

    TriangleVertex output;
    output.position = float4(positions[vertexID], 0.0, 1.0);
    output.color = colors[vertexID];
    return output;
}

fragment float4 triangle_fragment(TriangleVertex input [[stage_in]])
{
    return float4(input.color, 1.0);
}
