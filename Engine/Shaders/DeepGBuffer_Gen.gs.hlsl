#include "DeepGBuffer_Gen_InputStructs.hlsl"
#define G_BufferLayers 2

// Geometry shader
[maxvertexcount(G_BufferLayers * 3)]
void GSMain(triangle VertexOutput In[3], inout TriangleStream<PixelInput> triStream)
{
    PixelInput output;
    [unroll]
    for (uint i = 0; i < G_BufferLayers; i++)
    {
        // Select the slice based on the current G-Buffer layer
        #ifdef GEN_REPROJECT
        output.renderTargetIndex = i;
        #endif
        [unroll]
        for (int j = 0; j < 3; j++)
        {
            VertexOutput input = In[j];
            output.csNormal = input.csNormal;
            output.csTangent = input.csTangent;
            output.csBitangent = input.csBitangent;
            output.position = input.position;
            output.texCoord = input.texCoord;
            output.csPosition = input.csPosition;
            output.csPrevPosition = input.csPrevPosition;

            triStream.Append(output);
        }

        triStream.RestartStrip();
    }
}