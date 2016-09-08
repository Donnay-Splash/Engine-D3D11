#ifndef _PLANECLASS_H_
#define _PLANECLASS_H_


#include <d3d11.h>
#include <DirectXMath.h>
#include <math.h>
#include "meshclass.h"

class PlaneClass : public MeshClass
{
private:

	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};

public:
	PlaneClass();
	PlaneClass(const PlaneClass&);
	~PlaneClass();

	bool Initialize(ID3D11Device*, WCHAR*);
	
private:
	bool InitializeBuffers(ID3D11Device*);

	int m_planeWidth, m_planeHeight, m_planeDepth;

};

#endif