#define RS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), RootConstants(num32BitConstants=16, b0), DescriptorTable(SRV(t0)),\
                                                            StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_LINEAR, maxAnisotropy = 16, BorderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK)" 
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};
SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);
            
[RootSignature(RS)]
float4 PSMain(PS_INPUT input) : SV_Target
{
    float4 out_col = input.col * texture0.Sample(sampler0, input.uv);
    return out_col;
};