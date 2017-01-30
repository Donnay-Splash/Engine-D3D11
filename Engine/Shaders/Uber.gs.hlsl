#include "InputStructs.hlsl"
#define G_BufferLayers 2

// Geometry shader
[maxvertexcount(G_BufferLayers * 3)]
void GSMain(triangle VertexOut In[3], inout TriangleStream<GeometryOut> triStream)
{
    GeometryOut output;
    [unroll]
    for (uint i = 0; i < G_BufferLayers; i++)
    {
        [unroll]
        for (int j = 0; j < 3; j++)
        {
            VertexOut input = In[j];
            output.cameraPosition = input.cameraPosition;
            output.normal = input.normal;
            output.position = input.position;
            output.tex = input.tex;
            output.worldSpacePosition = input.worldSpacePosition;

            // Select the slice based on the current G-Buffer layer
            output.RTIndex = i;

            triStream.Append(output);
        }

        triStream.RestartStrip();
    }
}