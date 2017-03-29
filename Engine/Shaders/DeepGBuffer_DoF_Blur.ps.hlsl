/*  Splits the scene into near and far fields depending on depth and DoF settings*/

#include "PostEffectConstants.hlsl"

// As input takes packed colour and CoC alongside near and far field images
Texture2D packedColourTexture : register(t0);
Texture2D nearFieldTexture : register(t1);
Texture2D farFieldTexture : register(t2);
SamplerState dofSampler : register(s0);

cbuffer DoFConstants : register(b7)
{
    float dofEnabled;
    float focusPlaneZ;
    float scale;
    float maxCoCNear;
    float maxCoCFar;
}

struct PixelOut
{
    float4 halfResNearField;
    float4 halfResFarField;
};

PixelOut PSMain(VertexOut input)
{
    PixelOut result;

    // Sample CoC for both near and far field

    // Calculate size of blur kernel for both near and far field

    // Blur near and far field images using kernels
    // This will include overblurring in near field using alpha to control visibility at object edges

    // Return result ready for blur in other axis
    return result;
}