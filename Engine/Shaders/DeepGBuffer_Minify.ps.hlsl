#include "PostEffectConstants.hlsl"

#ifdef MINIFY_CSZ
Texture2D csZTexture : register(t0);
SamplerState csZSampler : register(s0);
#endif

#ifdef MINIFY_LAMBERT
Texture2D LambertOnlyLayer1Tex : register(t0);
Texture2D LambertOnlyLayer2Tex : register(t1);
Texture2D PackedNormalsTex : register(t2);
SamplerState LambertSampler : register(s0);
#endif

struct PixelOut
{
#ifdef MINIFY_CSZ
    float2 csZ : SV_Target;
#endif
#ifdef MINIFY_LAMBERT
    float4 lambertLayer1 : SV_Target0;
    float4 lambertLayer2 : SV_Target1;
    float4 packedNormal : SV_Target2;
#endif
};

PixelOut PSMain(VertexOut input)
{
    PixelOut result;

    int2 fragCoord = int2(input.position.xy);
    uint width, height, levels;
#ifdef MINIFY_CSZ
    csZTexture.GetDimensions(uint(0), width, height, levels);
#endif
#ifdef MINIFY_LAMBERT
    LambertOnlyLayer1Tex.GetDimensions(uint(0), width, height, levels);
#endif

    int2 samplePoint = clamp(2 * fragCoord + int2(fragCoord.y & 1, fragCoord.x & 1), 0, int2(width, height) - int2(1, 1));

#ifdef MINIFY_CSZ
    result.csZ = csZTexture.Load(int3(samplePoint, 0)).rg;
#endif
#ifdef MINIFY_LAMBERT
    result.lambertLayer1 = LambertOnlyLayer1Tex.Load(int3(samplePoint, 0));
    result.lambertLayer2 = LambertOnlyLayer2Tex.Load(int3(samplePoint, 0));
    result.packedNormal = PackedNormalsTex.Load(int3(samplePoint, 0));
#endif

    return result;
}