// GBuffer layout
Texture2DArray diffuseColor : register(t0);
Texture2DArray csNormals : register(t1);
Texture2DArray ssVelocity : register(t2);
Texture2DArray depth : register(t3);
SamplerState gBufferSampler : register(s0);

cbuffer Constants : register(b0)
{
    float displaySecondLayer;
    float gBufferTarget;
}

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 PSMain(VertexOut input) : SV_Target
{
    float4 color = 0.0f.xxxx;
    float target = floor(gBufferTarget);
    if (target == 0.0)
    {
        color = diffuseColor.Sample(gBufferSampler, float3(input.uv, displaySecondLayer));
    }
    else if (target == 1.0f)
    {
        color = csNormals.Sample(gBufferSampler, float3(input.uv, displaySecondLayer));
    }
    else if (target == 2.0f)
    {
        float2 ssVel = ssVelocity.Sample(gBufferSampler, float3(input.uv, displaySecondLayer));
        color = float4((ssVel + 1.0f) * 0.5f, 0.5f, 1.0f);
    }
    else if(target == 3.0f)
    {
        color = 1.0f.xxxx - depth.Sample(gBufferSampler, float3(input.uv, displaySecondLayer)).xxxx;
    }

    return pow(color, 1/2.2f);
}