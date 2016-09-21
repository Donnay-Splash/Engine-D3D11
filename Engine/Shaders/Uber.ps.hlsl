#include "Globals_ps.hlsl"

float4 PSMain(PixelInputType input) : SV_TARGET
{
    return float4((input.normal + 1) / 2.0f, 1.0f);
}


