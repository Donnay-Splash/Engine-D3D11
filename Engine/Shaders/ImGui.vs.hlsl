#define RS "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), RootConstants(num32BitConstants=16, b0), DescriptorTable(SRV(t0)),\
                                                            StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_LINEAR, maxAnisotropy = 16, BorderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK)" 

cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};
struct VS_INPUT
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
            
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

[RootSignature(RS)]
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}