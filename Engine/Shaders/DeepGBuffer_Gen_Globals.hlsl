/*  Contains all global constants for each pipeline stage used in deep G-Buffer generation */

// Stores the view and projection transform matrices.
// TODO: These can be concatinated into a viewProjectionMatrix.
// TODO: Require previous viewProjection to calculate velocity vectors
cbuffer ViewBuffer : register(b0)
{
    // Matrices are set to row_major to match DirectXMath
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    float3 cameraPos;
};

// Stores object information. e.g. object local to world transform.
// TODO: Require previous transform to calculate velocity vectors
cbuffer ObjectBuffer : register(b1)
{
    row_major matrix objectToCameraTransform;
    row_major matrix prevObjectToCameraTransform;
}