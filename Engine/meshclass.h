////////////////////////////////////////////////////////////////////////////////
// Filename: meshclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MESHCLASS_H_
#define _MESHCLASS_H_


//////////////
// INCLUDES //
//////////////
#include "textureclass.h"

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
	void Render(ID3D11DeviceContext*);
	bool LoadTexture(ID3D11Device*, WCHAR*);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTexture();
	int GetIndexCount();
	
private:
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	
	std::shared_ptr<TextureClass> m_Texture;
	
protected:	
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	int m_vertexCount;
	int	m_indexCount;
};

#endif