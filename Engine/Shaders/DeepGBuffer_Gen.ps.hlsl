#include "DeepGBuffer_Gen_Globals.hlsl"
#include "DeepGBuffer_Gen_InputStructs.hlsl"
#include "ReconstructFromDepth.hlsl"

// Need Previous depth buffer

// G_Buffer layout
struct PixelOutput
{
    float4 diffuseColor : SV_Target0;
    // Specular Color and smoothness
    float4 csNormal : SV_Target1;
    float2 ssVelocity : SV_Target2;
    float csZ : SV_Target3;
};

/*-------------------------------
Gets the screen space velocity of the current fragment.

Args:
csPosition: Current camera-space position
csPrevPosition: Previous camera-space position
---------------------------------*/
float2 GetSSVelocity(float4 csPosition, float4 csPrevPosition)
{
    // Project previous and current camera coordinates on the the screen.
    // Since the fragment coordinate will have already been rounded to pixel
    // centre this must also be done for the current position
    float4 ssPrevPosition = mul(csPrevPosition, projectionMatrix);
    float4 ssPosition = mul(csPosition, projectionMatrix);

    // Fit x,y coordinates of new screen space positions into single float4.
    // Remembering to reproject by dividing by w.
    float4 temp = float4(ssPrevPosition.xy, ssPosition.xy) / float4(ssPrevPosition.ww, ssPosition.ww);

    // Subtract old coordinates from new coordinates.
    return temp.zw - temp.xy;
}

/*--------------------------------
Checks to see if the currently rendered fragment is infront of the second layer. If so then it is discarded.

Args:
ssVelocity: The current velocity of the pixel in screen space. i.e. How much it has moved from last frame in screen coordinates
minSeparation: What is the minimum separation we want between first and second layer of deep G-Buffer
fragCoord: Screen space position of the fragment we are shading
prevZ: The previous Z value of the fragment
--------------------------------*/
bool IsInFrontOfSecondLayer(float2 ssVelocity, float minSeparation, float2 fragCoord, float prevZ)
{
    // Get previous frag coord
    float2 ssPrevCoord = (fragCoord * invViewSize) - ssVelocity;

    // Sample old depth // Need to convert location to float3 since we are sampling an array
    // ssPrevCoord is given as viewport coordinates. currentToPrevScale needs to be (1/width, 1/height)
    float oldDepth = previousDepth.SampleLevel(depthSampler, float3(ssPrevCoord, 0.0f), 0).r;

    // Reconstruct Z from old depth
    float oldZ = ReconstructCSZ(oldDepth, clipInfo);

    // Check to see if the current pixel is behind the previously rendered pixel.
    // If so then we want to discard it.
    return oldZ + minSeparation >= prevZ;
}

// TODO: need to uberise this to avoid invalid shader code.
PixelOutput PSMain(PixelInput input)
{
    // Calculate screen space velocity
    float2 ssVelocity = GetSSVelocity(input.csPosition, input.csPrevPosition);
    float2 currentToPrevScale = 1.0f.xx;

    // Discard pixels for second layer
    if (input.renderTargetIndex == 1 && IsInFrontOfSecondLayer(ssVelocity, minSeparation, input.position.xy, input.csPrevPosition.z))
    {
        discard;
    }

    // Ouput values to g_buffer
    PixelOutput output;
    output.diffuseColor = material_diffuseColorAndOpacity;
    output.diffuseColor.a = material_specularColorAndSmoothness.a;
    [branch]
    if(material_hasDiffuseTexture)
    {
        output.diffuseColor.rgb = DiffuseTexture.Sample(DiffuseSampler, input.texCoord).rgb;
    }
    [branch]
    if(material_hasSpecularTexture)
    {
        output.diffuseColor.a = SpecularTexture.Sample(SpecularSampler, input.texCoord).r;
    }

    float3 normal = normalize(input.csNormal);
    [branch]
    if(material_hasNormalTexture)
    {
        float3 tangent = normalize(input.csTangent);
        float3 bitangent = normalize(input.csBitangent);
        // Construct normal/tangent/bitangent matrix here.
        // where normal = z, tangent = x, bitangent = y
        //float handedness = step(0.0f, dot(cross(normal, tangent), bitangent));
        matrix<float, 3, 3> normalTransform ={  tangent,
                                                -bitangent,
                                                normal };

        // Sample texture and multiply by matrix
        float3 sampledNormal = NormalTexture.Sample(NormalSampler, input.texCoord).rgb;
        // convert normal from [0, 1] to [-1, 1]
        sampledNormal = normalize((sampledNormal * 2.0f) - 1.0f);
        normal = mul(sampledNormal, normalTransform);
        //normal = lerp(float3(0.0f, 0.0f, 1.0f), float3(0.0f, 1.0f, 0.0f), handedness);
    }

    float3 normalAsColor = (normalize(normal) + 1.0f.xxx) * 0.5f;
    output.csNormal = float4(normalAsColor, 1.0f);
    output.ssVelocity = ssVelocity;
    output.csZ = input.csPosition.z;
    return output;
}