#include "DeepGBuffer_Gen_InputStructs.hlsl"
#define G_BufferLayers 2

// Geometry shader
[maxvertexcount(G_BufferLayers * 3)]
void GSMain(triangle GeometryInput In[3], inout TriangleStream<PixelInput> triStream)
{
    PixelInput output;
    [unroll]
    for (uint i = 0; i < G_BufferLayers; i++)
    {
        // Select the slice based on the current G-Buffer layer
        output.renderTargetIndex = i;
        [unroll]
        for (int j = 0; j < 3; j++)
        {
            GeometryInput input = In[j];
            output.csNormal = input.csNormal;
            output.position = input.position;
            output.texCoord = input.texCoord;
            output.csPosition = input.csPosition;
            output.csPrevPosition = input.csPrevPosition;

            triStream.Append(output);
        }

        triStream.RestartStrip();
    }
}