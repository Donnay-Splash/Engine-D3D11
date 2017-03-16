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

/*--------------------------------------
Reconstructs the camera-space normal for the face based on screen-space derivatives

Args:
csPosition: camera-space position for the fragment
--------------------------------------*/
float3 ReconstructNonUnitFaceNormal(float3 csPosition)
{
    return cross(ddx(csPosition), ddy(csPosition));
}

/*--------------------------------------
Packs normalised Z into two 8 bit channels of RGBA texture. Allowing for greated depth precision.

Args:
key: normalised camera-space depth
--------------------------------------*/
float2 PackBilateralKey(float key)
{
    float2 result;
    float temp = floor(key * 255.0f);
    result.r = temp / 255.0f;
    result.g = (key * 255.0f) - temp;

    return result;
}

/*--------------------------------------
Unpacks normalised camera-space depth from 2 8 bit normalised channels

Args:
packedKey: packed depth
--------------------------------------*/
float UnpackBilateralKey(float2 packedkey)
{
    return packedkey.x + packedkey.y * (1.0f / 255.0f);
}

/*--------------------------------------
Packs a float3 normal into 2 channels using spheremap transform.
http://aras-p.info/texts/CompactNormalStorage.html#method04spheremap

Note we use negative Z as we are working in screen space and all normals are pointing towards the camera
This gives us a positive value for the sqrt which seems to produce more accurate results.

Args:
normal: normalised normal that we want to pack
--------------------------------------*/
float2 PackNormal(float3 normal)
{
    float p = sqrt(-normal.z * 8 + 8);
    return float2(normal.xy/p + 0.5f);
}

/*--------------------------------------
Reconstructs a packed normal using spheremap transform
http://aras-p.info/texts/CompactNormalStorage.html#method04spheremap

When encoding we negated the Z coordinate so to revert this we again invert the z coordinate to
return to the original value. This appears to improve quality.

Args:
packedNormal: the 2 channel packed normal we want to reconstruct
--------------------------------------*/
float3 UnpackNormal(float2 packedNormal)
{
    float2 fenc = packedNormal * 4.0f - 2.0f;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0f - f/4.0f);
    float3 normal;
    normal.xy = fenc * g;
    normal.z = -(1.0f - f/2.0f);
    return normal;
}

