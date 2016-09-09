#include "pch.h"
#include "planeclass.h"

PlaneClass::PlaneClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}

PlaneClass::PlaneClass(const PlaneClass& other)
{
}

PlaneClass::~PlaneClass()
{
}

bool PlaneClass::Initialize(ID3D11Device* device, WCHAR* textureFilename)
{
	bool result;

	// Manually set the width and height of the terrain.
	m_planeWidth = 100;
	m_planeHeight = 100;
	m_planeDepth = 100;

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	// Load the texture for this model.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


bool PlaneClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j, k;
	float positionX, positionY, positionZ;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;


	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_planeWidth - 1) * (m_planeHeight - 1) * (m_planeDepth - 1) * 8;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if(!indices)
	{
		return false;
	}

	// Initialize the index to the vertex array.
	index = 0;

	// Load the vertex and index arrays with the terrain data.
	for(j=0; j<(m_planeHeight-1); j++)
	{
		for(i=0; i<(m_planeWidth-1); i++)
		{
			// Upper left.
			positionX = (float)i;
			positionZ = (float)(j);


			vertices[index].position = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
			vertices[index].texture = Utils::Maths::Vector2(0.0, 0.0);
			vertices[index].normal = Utils::Maths::Vector3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Upper right.
			positionX = (float)(i);
			positionZ = (float)(j+1);

			vertices[index].position = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
			vertices[index].texture = Utils::Maths::Vector2(0.0, 0.0);
			vertices[index].normal = Utils::Maths::Vector3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// lower left
			positionX = (float)(i+1);
			positionZ = (float)(j);

			vertices[index].position = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
			vertices[index].texture = Utils::Maths::Vector2(0.0, 0.0);
			vertices[index].normal = Utils::Maths::Vector3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Bottom right.  lower left
			positionX = (float)(i);
			positionZ = (float)(j+1);

			vertices[index].position = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
			vertices[index].texture = Utils::Maths::Vector2(0.0, 0.0);
			vertices[index].normal = Utils::Maths::Vector3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Bottom right.  upper right
			positionX = (float)(i+1);
			positionZ = (float)(j+1);

			vertices[index].position = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
			vertices[index].texture = Utils::Maths::Vector2(0.0, 0.0);
			vertices[index].normal = Utils::Maths::Vector3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Bottom rigjht.
			positionX = (float)(i+1);
			positionZ = (float)(j);

			vertices[index].position = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
			vertices[index].texture = Utils::Maths::Vector2(0.0, 0.0);
			vertices[index].normal = Utils::Maths::Vector3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			
		}
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}


