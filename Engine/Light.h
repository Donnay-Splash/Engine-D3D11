#pragma once

class Light
{
public:
    Light();
    ~Light();

    void SetDiffuseColor(Utils::Maths::Color);
    void SetDirection(Utils::Maths::Vector3);
    void SetAmbientColor(Utils::Maths::Color);
    void SetSpecularColor(Utils::Maths::Color);
    void SetSpecularPower(float);
    void SetPosition(Utils::Maths::Vector3);

    Utils::Maths::Color GetDiffuseColor();
    Utils::Maths::Vector3 GetDirection();
    Utils::Maths::Color GetAmbientColor();
    Utils::Maths::Color GetSpecularColor();
    Utils::Maths::Vector3 GetPosition();
    float GetSpecularPower();

private:
    Utils::Maths::Color m_diffuseColor;
    Utils::Maths::Vector3 m_direction;
    Utils::Maths::Color m_ambientColor;
    Utils::Maths::Color m_specularColor;
    Utils::Maths::Vector3 m_position;
    float m_specularPower;
};

