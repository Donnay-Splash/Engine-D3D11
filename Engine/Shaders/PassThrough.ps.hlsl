#define RS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), DescriptorTable(CBV(b0))" 

cbuffer SceneConstantBuffer : register(b0)
{
	float4 offset;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

[RootSignature(RS)]
float4 PSMain( PSInput input ) : SV_TARGET
{
	return offset;
}