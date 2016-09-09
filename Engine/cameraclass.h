////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


//////////////
// INCLUDES //
//////////////
#include "Math.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: CameraClass
////////////////////////////////////////////////////////////////////////////////
class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(Utils::Maths::Vector3 position);
	void SetRotation(Utils::Maths::Quaternion rotation);

    Utils::Maths::Vector3 GetPosition() { return m_position; }
    Utils::Maths::Quaternion GetRotation() { return m_rotation; }

	void Render();
    Utils::Maths::Matrix GetViewMatrix() { return m_viewMatrix; }

private:
    bool m_dirtyFlag = true;
	Utils::Maths::Vector3 m_position;
    Utils::Maths::Quaternion m_rotation;

	// Note: Given an identity matrix the camera will be centred at the origin
	// looking down the positive z axis with up on the y axis
	Utils::Maths::Matrix m_viewMatrix;
};

#endif