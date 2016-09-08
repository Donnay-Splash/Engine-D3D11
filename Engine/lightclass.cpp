#include "lightclass.h"


LightClass::LightClass()
{
}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}


void LightClass::SetDiffuseColor(Utils::Maths::Color color)
{
	m_diffuseColor = color;
}


void LightClass::SetDirection(Utils::Maths::Vector3 direction)
{
	m_direction = direction;
}

void LightClass::SetAmbientColor(Utils::Maths::Color color)
{
	m_ambientColor = color;
}

void LightClass::SetSpecularColor(Utils::Maths::Color color)
{
	m_specularColor = color;
}

void LightClass::SetSpecularPower(float power)
{
	m_specularPower = power;
}

void LightClass::SetPosition(Utils::Maths::Vector3 position)
{
    m_position = position;
}

Utils::Maths::Color LightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}


Utils::Maths::Vector3 LightClass::GetDirection()
{
	return m_direction;
}

Utils::Maths::Color LightClass::GetAmbientColor()
{
	return m_ambientColor;
}

Utils::Maths::Color LightClass::GetSpecularColor()
{
	return m_specularColor;
}

Utils::Maths::Vector3 LightClass::GetPosition()
{
	return m_position;
}

float LightClass::GetSpecularPower()
{
	return m_specularPower;
}