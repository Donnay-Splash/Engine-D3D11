////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(Utils::Maths::Vector3 position)
{
    m_position = position;
    m_dirtyFlag = true;
	return;
}


void CameraClass::SetRotation(Utils::Maths::Quaternion rotation)
{
    m_rotation = rotation;
    m_dirtyFlag = true;
	return;
}


void CameraClass::Render()
{
    if (m_dirtyFlag)
    {
        // Since the viewmatrix is composed of only rotations and translations it is orthogonal
        m_viewMatrix = Utils::Maths::Matrix::CreateFromTranslationRotationScale(m_position, m_rotation, 1.0f);
        m_viewMatrix = m_viewMatrix.GetInverse();
        m_dirtyFlag = false;
    }
}

