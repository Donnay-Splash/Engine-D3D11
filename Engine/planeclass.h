#ifndef _PLANECLASS_H_
#define _PLANECLASS_H_


#include "meshclass.h"

class PlaneClass : public MeshClass
{
private:

	struct VertexType
	{
		Utils::Maths::Vector3 position;
        Utils::Maths::Vector2 texture;
        Utils::Maths::Vector3 normal;
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