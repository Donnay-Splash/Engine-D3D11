cbuffer DoFConstants : register(b7)
{
    float dofEnabled;
    float nearBlurryPlaneZ;
    float nearSharpPlaneZ;
    float farSharpPlaneZ;

    float farBlurryPlaneZ;
    float nearScale;
    float farScale;
    float maxCoCRadiusPixels;

    float nearFieldBlurRadius;
    float invNearFieldBlurRadius;
    float farFieldRescale;
    float dofPad;

    float2 CoCTextureDimensions;
}