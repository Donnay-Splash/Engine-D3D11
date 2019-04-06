#define RS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)"

struct PSInput
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

struct VSInput
{
	float4 position : POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD;
};

[RootSignature(RS)]
PSInput VSMain(VSInput input)
{
	PSInput output;

	output.position = input.position;
	output.colour = float4(0.0f, input.uv, 1.0f);

	return output;
}