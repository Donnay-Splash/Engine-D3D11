cbuffer MatrixBuffer
{
	// Matrices are set to row_major to match DirectXMath
    row_major matrix worldMatrix;
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
};

cbuffer TimeBuffer
{
	float time;
	float height;
	float length;
};

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
	output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = input.tex;
	output.normal = input.normal;

    return output;
}