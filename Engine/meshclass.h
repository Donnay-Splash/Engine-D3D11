////////////////////////////////////////////////////////////////////////////////
// Filename: meshclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MESHCLASS_H_
#define _MESHCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include "textureclass.h"
#include "Math.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: MeshClass
////////////////////////////////////////////////////////////////////////////////
class MeshClass
{
private:

	struct VertexType
	{
		Utils::Maths::Vector3 position;
        Utils::Maths::Vector2 texture;
        Utils::Maths::Vector3 normal;
	};

public:
	MeshClass();
	MeshClass(const MeshClass&);
	~MeshClass();

	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool LoadTexture(ID3D11Device*, WCHAR*);
	ID3D11ShaderResourceView* GetTexture();
	int GetIndexCount();
	
private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	
	void ReleaseTexture();
	TextureClass* m_Texture;
	
protected:
	
	
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
};

#endif