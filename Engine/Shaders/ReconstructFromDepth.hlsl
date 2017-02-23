/*--------------------------------------
Taken from G3D engine by Morgan McGuire http://g3d.sourceforge.net/

Includes multiple functions to reconstruct values from the depth buffer.
----------------------------------------*/

/*--------------------------------------
Reconstructs linear Z from depth using camera clipping info.

Args:
depth: Sampled depth
clipInfo = Vector3(near * far, near - far, far);
----------------------------------------*/
float ReconstructCSZ(float depth, float3 clipInfo)
{
    return clipInfo.x / (clipInfo.y * depth + clipInfo.z);
}

/*--------------------------------------
Reconstructs camera-space position from screen-space (x, y) in pixels
and camera space z > 0.

Args:
ssPosition: Screen-space position of current fragment in pixels
csZ: Linear camera-space z position
projectionInfo = Vector4(2.0f / (width*P[0][0]), 
                         -2.0f / (height*P[1][1]),
                         -( 1.0f - P[0][2]) / P[0][0], 
                         ( 1.0f + P[1][2]) / P[1][1])
----------------------------------------*/
float3 ReconstructCSPosition(float2 ssPosition, float csZ, float4 projectionInfo)
{
    return float3((ssPosition.xy * projectionInfo.xy + projectionInfo.zw) * csZ, csZ);
}
