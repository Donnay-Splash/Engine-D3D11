// Helper class to store all vertex shader global constants for use in the uber shader
#include "InputStructs.hlsl"

// Stores the view and projection transform matrices.
// TODO: These can be concatinated into a viewProjectionMatrix.
cbuffer ViewBuffer : register(b0)
{
    // Matrices are set to row_major to match DirectXMath
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    float3 cameraPos;
};

// Stores object information. e.g. object local to world transform.
cbuffer ObjectBuffer : register(b1)
{
    row_major matrix objectTransform;
}
