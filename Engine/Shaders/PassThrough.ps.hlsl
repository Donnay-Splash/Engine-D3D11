#define RS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)"

struct PSInput
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

[RootSignature(RS)]
float4 PSMain( PSInput input ) : SV_TARGET
{
	return input.colour;
}