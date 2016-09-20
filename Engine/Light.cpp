#include "pch.h"
#include "Light.h"


Light::Light()
{
}


Light::~Light()
{
}


void Light::SetDiffuseColor(Utils::Maths::Color color)
{
    m_diffuseColor = color;
}


void Light::SetDirection(Utils::Maths::Vector3 direction)
{
    m_direction = direction;
}

void Light::SetAmbientColor(Utils::Maths::Color color)
{
    m_ambientColor = color;
}

void Light::SetSpecularColor(Utils::Maths::Color color)
{
    m_specularColor = color;
}

void Light::SetSpecularPower(float power)
{
    m_specularPower = power;
}

void Light::SetPosition(Utils::Maths::Vector3 position)
{
    m_position = position;
}

Utils::Maths::Color Light::GetDiffuseColor()
{
    return m_diffuseColor;
}


Utils::Maths::Vector3 Light::GetDirection()
{
    return m_direction;
}

Utils::Maths::Color Light::GetAmbientColor()
{
    return m_ambientColor;
}

Utils::Maths::Color Light::GetSpecularColor()
{
    return m_specularColor;
}

Utils::Maths::Vector3 Light::GetPosition()
{
    return m_position;
}

float Light::GetSpecularPower()
{
    return m_specularPower;
}