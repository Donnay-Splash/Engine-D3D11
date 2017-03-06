/*  Contains all sytsem global constants accessible from any shader stage (as long as the buffer slots havent't been overwritten)
    Global constants consist of:
        ViewConstants : Constant values describing the camera
    
*/

cbuffer ViewBuffer : register(b0)
{
    // Matrices are set to row_major to match DirectXMath
    row_major matrix viewMatrix; // world to camera-space
    row_major matrix projectionMatrix; // camera to screen-space
    row_major matrix jitteredProjectionMatrix; // Camera to screen-space with temporal jitter applied
    float4 projectionInfo; // Projection matrix info for reconstructing camera-space positions
    float3 cameraPos; // position of the camera in world space
    float padding;
    float3 clipInfo; // clippling plane info for reconstructing camera-space z from depth buffer
    float padding1;
    float2 invViewSize; // 1/viewport size
    float2 padding3;
    float projectionScale; // size of a 1m tall object 1m away from the camera in pixels
};