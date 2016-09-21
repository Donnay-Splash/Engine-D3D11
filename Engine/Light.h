#pragma once
#include "Math.h"
#include "Component.h"
// light information to be consumed by shader
// TODO: May need to add padding
struct LightData
{
    Utils::Maths::Vector3 Position;
    Utils::Maths::Vector3 Direction;
    Utils::Maths::Color Color;

    bool operator!=(const LightData& rhs)
    {
        return  (this->Position != rhs.Position) ||
            (this->Direction != rhs.Direction) ||
            (this->Color != rhs.Color);
    }

    bool operator==(const LightData& rhs)
    {
        return  (this->Position == rhs.Position) &&
            (this->Direction == rhs.Direction) &&
            (this->Color == rhs.Color);
    }
};

class Light : public Component
{
public:
    virtual void Update(float frameTime) override;
    virtual void Render(ID3D11DeviceContext* deviceContext) const override;

    void SetColor(Utils::Maths::Color);
    void SetDirection(Utils::Maths::Vector3);

    LightData GetLightData();

protected:
    Light(Component::SceneNodePtr sceneNode);
    virtual void Initialize(ID3D11Device* device) override;

private:
    LightData m_lightData;
    float m_specularPower;

    friend class SceneNode;
};

