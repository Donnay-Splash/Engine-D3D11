/*  Contains all sytsem global constants accessible from any shader stage (as long as the buffer slots havent't been overwritten)
    Global constants consist of:
        ViewConstants : Constant values describing the camera
    
*/

cbuffer ViewBuffer : register(b0)
{
    // Matrices are set to row_major to match DirectXMath
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    float4 projectionInfo;
    float3 cameraPos;
    float padding;
    float3 clipInfo;
    float padding2;
    float2 invViewSize;
};