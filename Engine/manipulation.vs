cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer TimeBuffer
{
	float time;
	float height;
	float length;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
	float x_comp = 0.0f;
	float z_comp = 0.0f;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// offset y position based on sine wave
	x_comp = height*sin(input.position.x * length + time); 
	z_comp = height*cos(input.position.z * length + time);
	input.position.y = x_comp * z_comp;
	
	// recalculate the normal
	input.normal.x = 1 - cos(input.position.x*0.125 + time);
	input.normal.y = abs(cos(input.position.x*0.125 + time));
	
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    
    // Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    return output;
}