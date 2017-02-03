#include "DeepGBuffer_Gen_InputStructs.hlsl"

// G_Buffer layout
struct PixelOutput
{
    float4 diffuseColor : SV_Target0;
    float4 csNormal : SV_Target1;
    float4 csVelocity : SV_Target2;
};

PixelOutput PSMain(PixelInput input)
{
    // Calculate screen space velocity

    // Discard pixels for second layer

    // Ouput values to g_buffer
    PixelOutput output;
    output.diffuseColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
    float3 normalAsColor = (normalize(input.csNormal) + 1.0f.xxx) * 0.5f;
    output.csNormal = float4(normalAsColor, 1.0f);
    return output;
}