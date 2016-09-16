cbuffer MatrixBuffer : register(b0)
{
    // Matrices are set to row_major to match DirectXMath
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
};

cbuffer TimeBuffer : register(b1)
{
    float time;
    float height;
    float length;
};

cbuffer ObjectBuffer : register(b2)
{
    row_major matrix objectTransform;
}

struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};


PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output;
    input.position.w = 1.0f;
    // Store the texture coordinates for the pixel shader.
    output.position = mul(input.position, objectTransform);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(sin(time), cos(time));
    output.normal = input.normal;

    return output;
}