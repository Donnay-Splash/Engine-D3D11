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